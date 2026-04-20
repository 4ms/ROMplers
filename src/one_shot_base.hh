#pragma once

#include "plugin.hpp"

struct SpeedQuantity : ParamQuantity {
  std::string getDisplayValueString() override {
    float v = getValue();
    float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
    return string::f("%.3gx", display);
  }
};

template <typename T> class OneShotBaseModule : public Module {
public:
  enum ParamId {
    SAMPLE_PARAM,
    PITCH_PARAM,
    DECAY_PARAM,
    PUSH_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SAMPLECVIN_INPUT,
    PITCHCVIN_INPUT,
    DECAYCVIN_INPUT,
    TRIGIN_INPUT,
    VOLCVIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId { AUDIOOUT_OUTPUT, OUTPUTS_LEN };
  enum LightId { LIGHT, LIGHTS_LEN };

  float samplePos = 0.f;
  bool playing = false;

  uint32_t cur_sample_idx{};

  float env = 0.f;
  float lastTrigValue = 0.f;
  float lastButtonValue = 0.f;
  float LightBrightness = 0.f;

  static constexpr float MIN_PLAYBACK_SPEED = T::min_rate;
  static constexpr float MAX_PLAYBACK_SPEED = T::max_rate;

  OneShotBaseModule() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    std::vector<std::string> sampleChoices;
    for (auto i = 1u; i <= T::samples.size(); ++i)
      sampleChoices.push_back(std::to_string(i));
    configSwitch(SAMPLE_PARAM, 0.f, (T::samples.size() - 1), 0.f, "Sample",
                 sampleChoices);
    configParam<SpeedQuantity>(PITCH_PARAM, -1.f, 1.f, 0.f, "Pitch");
    configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
    configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");
    configInput(SAMPLECVIN_INPUT, "Sample CV");
    configInput(PITCHCVIN_INPUT, "Pitch CV");
    configInput(DECAYCVIN_INPUT, "Decay CV");
    configInput(VOLCVIN_INPUT, "Volume CV");
    configInput(TRIGIN_INPUT, "Trig");
    configOutput(AUDIOOUT_OUTPUT, "Audio output");
  }

  void process(const ProcessArgs &args) override {
    float trigIn = inputs[TRIGIN_INPUT].getVoltage();
    float buttonIn = params[PUSH_PARAM].getValue();

    bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
    bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

    lastTrigValue = trigIn;
    lastButtonValue = buttonIn;

    bool triggered = trigRising || buttonRising;

    if (triggered) {
      LightBrightness = 1.0f;

      float sampleCV = std::clamp(inputs[SAMPLECVIN_INPUT].getVoltage(), -5.f, 5.f);
      int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() +
                                   sampleCV / 5.f * 8.f);
      cur_sample_idx =
          (uint32_t)std::clamp<int>(sampleIndex, 0, (int)T::samples.size() - 1);

      samplePos = 0.f;
      playing = true;
      env = 1.0f;
    }

    LightBrightness =
        std::max(0.f, LightBrightness - (float)(args.sampleTime * 10.f));
    lights[LIGHT].setBrightnessSmooth(LightBrightness, args.sampleTime);

    float output = 0.f;

    if (playing) {
      // --- PITCH ---
      // Knob (-1..1) rescaled to -5..5V offset; CV added and clamped to ±5V
      const float knobPitchOffset = params[PITCH_PARAM].getValue() * 5.f;
      const float pitchCV = inputs[PITCHCVIN_INPUT].isConnected()
                                ? inputs[PITCHCVIN_INPUT].getVoltage()
                                : 0.f;
      float pitchMod = rescale(std::clamp(knobPitchOffset + pitchCV, -5.f, 5.f), -5.f, 5.f, -1.f, 1.f);

      float normalizedPitch = (pitchMod + 1.f) * 0.5f;
      float pitchRatio =
          MIN_PLAYBACK_SPEED +
          normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);

      // --- DECAY ---
      // Knob (0..1) rescaled to -5..5V offset; CV added and clamped to ±5V
      const float knobDecayOffset = rescale(params[DECAY_PARAM].getValue(), 0.f, 1.f, -5.f, 5.f);
      const float decayCV = inputs[DECAYCVIN_INPUT].isConnected()
                                ? inputs[DECAYCVIN_INPUT].getVoltage()
                                : 0.f;
      float decayMod = rescale(std::clamp(knobDecayOffset + decayCV, -5.f, 5.f), -5.f, 5.f, 0.f, 1.f);

      static constexpr auto minDecayTime = 0.005f;
      const auto maxDecayTime = T::samples[cur_sample_idx].size() / 44100.f;
      const auto decayTime =
          minDecayTime + decayMod * (maxDecayTime - minDecayTime);
      const auto decayCoef = expf(-1.f / (decayTime * args.sampleRate));

      // --- Sample playback ---
      samplePos += pitchRatio * (44100.f / args.sampleRate);

      if ((uint32_t)samplePos >= T::samples[cur_sample_idx].size()) {
        playing = false;
      } else {
        auto idx = (uint32_t)samplePos;
        auto nextIdx =
            (idx + 1 < T::samples[cur_sample_idx].size()) ? idx + 1 : idx;
        auto frac = samplePos - idx;
        auto sampleValue = T::samples[cur_sample_idx][idx] +
                           frac * (T::samples[cur_sample_idx][nextIdx] -
                                   T::samples[cur_sample_idx][idx]);
        env *= decayCoef;
        output = sampleValue * env;
      }
    }

    float volumeCV = 5.f;
    if (inputs[VOLCVIN_INPUT].isConnected()) {
      volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
    }

    output *= volumeCV / 5.f;

    outputs[AUDIOOUT_OUTPUT].setVoltage(output * 10.0f);
  }
};

template <typename T, typename M>
class OneShotBaseWidget : public ModuleWidget {
public:
  OneShotBaseWidget(M *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, T::panel)));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module,
                                          M::SAMPLE_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module,
                                          M::PITCH_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module,
                                          M::DECAY_PARAM));
    addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module,
                                           M::PUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(10.16, 84.3)), module, M::LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module,
                                             M::SAMPLECVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module,
                                             M::PITCHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module,
                                             M::DECAYCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module,
                                             M::TRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module,
                                             M::VOLCVIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module,
                                               M::AUDIOOUT_OUTPUT));
  }
};
