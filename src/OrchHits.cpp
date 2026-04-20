#include "OrchHitsSamples.hpp"
#include "dsp_utils.hh"
#include "cv_func.hh"
#include "plugin.hpp"
#include "sample.hh"
#include <cmath>

struct OrchHits : Module {
  enum ParamId {
    SAMPLE_PARAM,
    OCTAVE_PARAM,
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
    OCTAVECVIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId { AUDIOOUT_OUTPUT, OUTPUTS_LEN };
  enum LightId { ORCHHITS_LIGHT, LIGHTS_LEN };

  float samplePos = 0.f;
  bool playing = false;

  float env = 0.f;
  float lastTrigValue = 0.f;
  float lastButtonValue = 0.f;
  float OrchHitsLightBrightness = 0.f;

  static constexpr float sampleSampleRate = 44100.f;

  static constexpr std::array samples = {
      Sample{Orch1},  Sample{Orch2},  Sample{Orch3},  Sample{Orch4},
      Sample{Orch5},  Sample{Orch6},  Sample{Orch7},  Sample{Orch8},
      Sample{Orch9},  Sample{Orch10}, Sample{Orch11}, Sample{Orch12},
      Sample{Orch13}, Sample{Orch14}, Sample{Orch15}, Sample{Orch16},
  };

  const float minDecayTime = 0.1f;
  const float maxDecayTime = 5.f;

  OrchHits() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    std::vector<std::string> sampleChoices;
    for (auto i = 1u; i <= samples.size(); ++i)
      sampleChoices.push_back(std::to_string(i));
    configSwitch(SAMPLE_PARAM, 0.f, (samples.size() - 1), 0.f, "Sample",
                 sampleChoices);
    configSwitch(OCTAVE_PARAM, 0.f, 4.f, 2.f, "Octave transpose",
                 {"-2", "-1", "Unison", "+1", "+2"});
    configParam(DECAY_PARAM, 0.01f, 5.f, 5.f, "Decay", "s");
    configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");

    configInput(SAMPLECVIN_INPUT, "Sample Select CV");
    configInput(OCTAVECVIN_INPUT, "Octave CV");
    configInput(PITCHCVIN_INPUT, "Pitch CV (1V/oct)");
    configInput(DECAYCVIN_INPUT, "Decay CV");
    configInput(TRIGIN_INPUT, "Trig");
    configInput(VOLCVIN_INPUT, "Volume CV");
    configOutput(AUDIOOUT_OUTPUT, "Audio output");
  }

  void process(const ProcessArgs &args) override {
    // --- SAMPLE SELECTION WITH CV ---
	int selectedIndex = indexed_cv_knob<samples.size()>(inputs[SAMPLECVIN_INPUT].getNormalVoltage(0), params[SAMPLE_PARAM].getValue());

    auto &s = samples[selectedIndex];

    float trigIn = inputs[TRIGIN_INPUT].getVoltage();
    float buttonIn = params[PUSH_PARAM].getValue();

    bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
    bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
    lastTrigValue = trigIn;
    lastButtonValue = buttonIn;

    if (trigRising || buttonRising) {
      OrchHitsLightBrightness = 1.f;
      samplePos = 0.f;
      playing = true;
      env = 1.f;
    }

    OrchHitsLightBrightness -= args.sampleTime * 10.f;
    if (OrchHitsLightBrightness < 0.f)
      OrchHitsLightBrightness = 0.f;
    lights[ORCHHITS_LIGHT].setBrightnessSmooth(OrchHitsLightBrightness,
                                               args.sampleTime);

    float output = 0.f;

    if (playing) {
      // --- OCTAVE CONTROL WITH QUANTIZED CV (±5V) ---
      float octaveKnob = params[OCTAVE_PARAM].getValue(); // 0 .. 4
      float octaveCV = inputs[OCTAVECVIN_INPUT].getNormalVoltage(0);

      // Scale CV: ±5V maps to full knob range (0..4)
      float cvScaled = octaveCV * 2.f / 5.f; // 5V → +2, -5V → -2

      // Quantize to discrete steps 0..4
      float totalOctave = std::clamp(std::round(octaveKnob + cvScaled), 0.f, 4.f);

      // standard 1V/oct, clamped to ±1V: 0V=unison, ±1V=±1oct
      float pitchCV = std::clamp(inputs[PITCHCVIN_INPUT].getNormalVoltage(0), -1.f, 1.f);
      float totalVolts = (totalOctave - 2.f) + pitchCV;
      float pitchRatio = calc1VperOctPitchRatio(totalVolts);

      float sampleRateRatio = sampleSampleRate / args.sampleRate;
      samplePos += pitchRatio * sampleRateRatio;

      const auto numSamplesInSample = s.size();

      if ((uint32_t)samplePos >= numSamplesInSample) {
        playing = false;
        output = 0.f;
      } else {
        auto idx = (uint32_t)samplePos;
        int nextIdx = (idx + 1 < numSamplesInSample) ? idx + 1 : idx;
        float frac = samplePos - idx;

        const auto s1 = s[idx];
        const auto s2 = s[nextIdx];

        const auto sampleValue = s1 + frac * (s2 - s1);

        const float cv = inputs[DECAYCVIN_INPUT].getNormalVoltage(0);
        const float decayParam = calcDecayModScaled(params[DECAY_PARAM].getValue(), cv);

        float decayTime =
            minDecayTime + decayParam * (maxDecayTime - minDecayTime);
        float decayCoef = std::exp(-5.f / (decayTime * args.sampleRate));
        env *= decayCoef;

        output = sampleValue * env;
      }
    }

    float volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getNormalVoltage(5.f), 0.f, 5.f);
    output *= volumeCV / 5.f;
    outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.f);
  }
};

struct OrchHitsWidget : ModuleWidget {
  OrchHitsWidget(OrchHits *module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/panels/OrchHits.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module,
                                          OrchHits::SAMPLE_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module,
                                          OrchHits::OCTAVE_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 62.001)), module,
                                          OrchHits::DECAY_PARAM));
    addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module,
                                           OrchHits::PUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(10.16, 84.3)), module, OrchHits::ORCHHITS_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module,
                                             OrchHits::SAMPLECVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 49.001)), module,
                                             OrchHits::OCTAVECVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 49.001)), module,
                                             OrchHits::PITCHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 74.701)), module,
                                             OrchHits::DECAYCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module,
                                             OrchHits::TRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module,
                                             OrchHits::VOLCVIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module,
                                               OrchHits::AUDIOOUT_OUTPUT));
  }
};

Model *modelOrchHits = createModel<OrchHits, OrchHitsWidget>("OrchHits");
