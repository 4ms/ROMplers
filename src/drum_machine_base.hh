#pragma once

#include "plugin.hpp"
#include "sample.hh"

struct SpeedQuantity : ParamQuantity {
  std::string getDisplayValueString() override {
    float v = getValue();
    float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
    return string::f("%.3gx", display);
  }
};

struct NamedSample {
  const char *name;
  Sample sample;
};

template <typename T> class DrumMachineBaseModule : public Module {
  struct Voice {
    bool lastInputTrigger = false;
    bool lastButtonTrigger = false;
    float samplePos = 0.f;
    bool playing = false;
  };

public:
  enum ParamId {
    SPEED_PARAM,
    LENGTH_PARAM,
    MAINVOL_PARAM,
    DRUM0_PARAM,
    DRUM1_PARAM,
    DRUM2_PARAM,
    DRUM3_PARAM,
    DRUM4_PARAM,
    DRUM5_PARAM,
    DRUM6_PARAM,
    DRUM7_PARAM,
    DRUM8_PARAM,
    DRUM9_PARAM,
    DRUM10_PARAM,
    DRUM11_PARAM,
  };

  static constexpr unsigned PARAMS_LEN = T::drums.size() + 3;

  enum InputId {
    SPEEDCVIN_INPUT,
    LENGTHCVIN_INPUT,
    DRUM0_INPUT,
    DRUM1_INPUT,
    DRUM2_INPUT,
    DRUM3_INPUT,
    DRUM4_INPUT,
    DRUM5_INPUT,
    DRUM6_INPUT,
    DRUM7_INPUT,
    DRUM8_INPUT,
    DRUM9_INPUT,
    DRUM10_INPUT,
    DRUM11_INPUT,
  };

  static constexpr unsigned INPUTS_LEN = T::drums.size() + 2;

  enum OutputId {
    DRUM0_OUTPUT,
    DRUM1_OUTPUT,
    DRUM2_OUTPUT,
    DRUM3_OUTPUT,
    DRUM4_OUTPUT,
    DRUM5_OUTPUT,
    DRUM6_OUTPUT,
    DRUM7_OUTPUT,
    DRUM8_OUTPUT,
    DRUM9_OUTPUT,
    DRUM10_OUTPUT,
    DRUM11_OUTPUT,
  };

  static constexpr unsigned SUM_OUTPUT = T::drums.size();
  static constexpr unsigned OUTPUTS_LEN = T::drums.size() + 1;

  enum LightId {
    DRUM0_LIGHT,
    DRUM1_LIGHT,
    DRUM2_LIGHT,
    DRUM3_LIGHT,
    DRUM4_LIGHT,
    DRUM5_LIGHT,
    DRUM6_LIGHT,
    DRUM7_LIGHT,
    DRUM8_LIGHT,
    DRUM9_LIGHT,
    DRUM10_LIGHT,
    DRUM11_LIGHT,
  };

  static constexpr unsigned LIGHTS_LEN = T::drums.size();

  static constexpr float SPEED_MIN = 0.05f;
  static constexpr float SPEED_MAX = 2.0f;
  static constexpr float LENGTH_MIN = 0.1f;
  static constexpr float LENGTH_MAX = 1.0f;

  std::array<Voice, 12> voice{};

  DrumMachineBaseModule() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
    configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
    configParam(MAINVOL_PARAM, 0.f, 1.f, 0.5f, "Main Volume", "%", 0.f, 100.f);

    for (auto i = 0u; i < T::drums.size(); ++i) {
      std::string v = std::string{T::drums[i].name} + " Trig";
      configSwitch(i + DRUM0_PARAM, 0.f, 1.f, 0.f, v, {"Off", "On"});
      configInput(i + DRUM0_INPUT, v);
      configOutput(i + DRUM0_OUTPUT, T::drums[i].name);
    }

    configInput(SPEEDCVIN_INPUT, "Speed CV");
    configInput(LENGTHCVIN_INPUT, "Length CV");
    configOutput(SUM_OUTPUT, "Sum");
  }

  void process(const ProcessArgs &args) override {
    // --- Speed ---
    const auto knobSpeed = (params[SPEED_PARAM].getValue() + 1.f) * 2.5f;
    auto speedCV = inputs[SPEEDCVIN_INPUT].isConnected()
                       ? inputs[SPEEDCVIN_INPUT].getVoltage()
                       : 0.f;
    speedCV = std::clamp(speedCV * 0.5f, -5.f, 5.f);
    const auto combinedSpeed = knobSpeed + speedCV;
    const auto normSpeed = std::clamp(combinedSpeed / 5.f, 0.f, 1.f);
    const auto speed = SPEED_MIN + normSpeed * (SPEED_MAX - SPEED_MIN);

    // --- Length ---
    const auto knobLength = params[LENGTH_PARAM].getValue() * 5.f;
    auto lengthCV = inputs[LENGTHCVIN_INPUT].isConnected()
                        ? inputs[LENGTHCVIN_INPUT].getVoltage()
                        : 0.f;
    lengthCV = std::clamp(lengthCV * 0.5f, -5.f, 5.f);
    const auto combinedLength = knobLength + lengthCV;
    const auto normLength = std::clamp(combinedLength / 5.f, 0.f, 1.f);
    const auto lengthRatio =
        LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);

    const auto mainVol = params[MAINVOL_PARAM].getValue();

    auto busSum{0.f};

    for (auto i = 0u; i < T::drums.size(); ++i) {
      const auto output_id = i + DRUM0_OUTPUT;
      const auto input_id = i + DRUM0_INPUT;
      const auto param_id = i + DRUM0_PARAM;
      const auto light_id = i + DRUM0_LIGHT;
      const Sample &samp = T::drums[i].sample;

      auto &v = voice[i];

      bool inTrig = inputs[input_id].getVoltage() > 1.f;
      bool btnTrig = params[param_id].getValue() > 0.5f;
      bool inputRising = inTrig && !v.lastInputTrigger;
      bool buttonRising = btnTrig && !v.lastButtonTrigger;

      bool fired = inputRising || buttonRising;

      if (fired) {
        v.playing = true;
        v.samplePos = 0.f;
      }

      v.lastInputTrigger = inTrig;
      v.lastButtonTrigger = btnTrig;

      const auto maxSamples = static_cast<unsigned>(samp.size() * lengthRatio);

      if (v.playing) {
        const auto idx = static_cast<uint32_t>(v.samplePos);
        if (idx < maxSamples) {
          const auto out = samp[idx];
          outputs[output_id].setVoltage(out * 5.f);
          v.samplePos += speed;
        } else {
          v.playing = false;
          outputs[output_id].setVoltage(0.f);
        }
      } else {
        outputs[output_id].setVoltage(0.f);
      }

      if (light_id >= 0) {
        if (fired)
          lights[light_id].setBrightness(1.f);
        else
          lights[light_id].setBrightnessSmooth(0.f, args.sampleTime);
      }

      if (!outputs[output_id].isConnected()) {
        busSum += outputs[output_id].getVoltage();
      }
    }

    outputs[SUM_OUTPUT].setVoltage(
        std::clamp(busSum / (float)T::drums.size() * mainVol * 10.f, -10.f, 10.f));
  }
};

template <typename T, typename W>
class DrumMachine12BaseWidget : public ModuleWidget {
public:
  DrumMachine12BaseWidget(W *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, T::panel)));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(16.799, 15.501)),
                                                   module, W::LENGTH_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(38.799, 15.501)),
                                                   module, W::SPEED_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(59.351, 15.501)),
                                                   module, W::MAINVOL_PARAM));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(8.798, 42.002)), module,
                                           W::DRUM0_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(8.798, 42.002)), module, W::DRUM0_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(20.5, 42.002)), module,
                                           W::DRUM1_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(20.5, 42.002)), module, W::DRUM1_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(32.3, 42.002)), module,
                                           W::DRUM2_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(32.3, 42.002)), module, W::DRUM2_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(43.998, 42.002)), module,
                                           W::DRUM3_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(43.998, 42.002)), module, W::DRUM3_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(55.7, 42.002)), module,
                                           W::DRUM4_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(55.7, 42.002)), module, W::DRUM4_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(67.501, 42.002)), module,
                                           W::DRUM5_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(67.501, 42.002)), module, W::DRUM5_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(8.798, 84.999)), module,
                                           W::DRUM6_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(8.798, 84.999)), module, W::DRUM6_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(20.5, 84.999)), module,
                                           W::DRUM7_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(20.5, 84.999)), module, W::DRUM7_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(32.3, 84.999)), module,
                                           W::DRUM8_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(32.3, 84.999)), module, W::DRUM8_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(43.998, 84.999)), module,
                                           W::DRUM9_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(43.998, 84.999)), module, W::DRUM9_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(55.7, 84.999)), module,
                                           W::DRUM10_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(55.7, 84.999)), module, W::DRUM10_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(67.501, 84.999)), module,
                                           W::DRUM11_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(67.501, 84.999)), module, W::DRUM11_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.849, 30.801)), module,
                                             W::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.851, 30.801)), module,
                                             W::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.798, 56.0)), module,
                                             W::DRUM0_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.5, 56.0)), module,
                                             W::DRUM1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.3, 56.0)), module,
                                             W::DRUM2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.998, 56.0)), module,
                                             W::DRUM3_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.7, 56.0)), module,
                                             W::DRUM4_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67.501, 56.0)), module,
                                             W::DRUM5_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.798, 99.001)), module,
                                             W::DRUM6_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.5, 99.001)), module,
                                             W::DRUM7_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.3, 99.001)), module,
                                             W::DRUM8_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.998, 99.001)), module,
                                             W::DRUM9_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.7, 99.001)), module,
                                             W::DRUM10_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67.501, 99.001)), module,
                                             W::DRUM11_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(59.351, 30.801)),
                                               module, W::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.798, 70.002)),
                                               module, W::DRUM0_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.5, 70.002)), module,
                                               W::DRUM1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.3, 70.002)), module,
                                               W::DRUM2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 70.002)),
                                               module, W::DRUM3_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.7, 70.002)), module,
                                               W::DRUM4_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(67.501, 70.002)),
                                               module, W::DRUM5_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.798, 112.999)),
                                               module, W::DRUM6_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.5, 112.999)),
                                               module, W::DRUM7_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.3, 112.999)),
                                               module, W::DRUM8_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.999)),
                                               module, W::DRUM9_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.7, 112.999)),
                                               module, W::DRUM10_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(67.501, 112.999)),
                                               module, W::DRUM11_OUTPUT));
  }
};

template <typename T, typename W>
class DrumMachine10BaseWidget : public ModuleWidget {
public:
  DrumMachine10BaseWidget(W *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, T::panel)));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 15.501)),
                                                   module, W::LENGTH_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(31.249, 15.501)),
                                                   module, W::SPEED_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(49.731, 15.501)),
                                                   module, W::MAINVOL_PARAM));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(6.999, 42.002)), module,
                                           W::DRUM0_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(6.999, 42.002)), module, W::DRUM0_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(18.75, 42.002)), module,
                                           W::DRUM1_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(18.75, 42.002)), module, W::DRUM1_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(30.501, 42.002)), module,
                                           W::DRUM2_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(30.501, 42.002)), module, W::DRUM2_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(42.249, 42.002)), module,
                                           W::DRUM3_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(42.249, 42.002)), module, W::DRUM3_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.0, 42.002)), module,
                                           W::DRUM4_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(54.0, 42.002)), module, W::DRUM4_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(6.999, 84.999)), module,
                                           W::DRUM5_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(6.999, 84.999)), module, W::DRUM5_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(18.75, 84.999)), module,
                                           W::DRUM6_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(18.75, 84.999)), module, W::DRUM6_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(30.501, 84.999)), module,
                                           W::DRUM7_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(30.501, 84.999)), module, W::DRUM7_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(42.249, 84.999)), module,
                                           W::DRUM8_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(42.249, 84.999)), module, W::DRUM8_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.0, 84.999)), module,
                                           W::DRUM9_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(54.0, 84.999)), module, W::DRUM9_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 30.801)), module,
                                             W::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.249, 30.801)), module,
                                             W::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.999, 56.0)), module,
                                             W::DRUM0_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.75, 56.0)), module,
                                             W::DRUM1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.501, 56.0)), module,
                                             W::DRUM2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.249, 56.0)), module,
                                             W::DRUM3_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.0, 56.0)), module,
                                             W::DRUM4_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.999, 99.001)), module,
                                             W::DRUM5_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.75, 99.001)), module,
                                             W::DRUM6_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.501, 99.001)), module,
                                             W::DRUM7_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.249, 99.001)), module,
                                             W::DRUM8_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.0, 99.001)), module,
                                             W::DRUM9_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.731, 30.801)),
                                               module, W::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.999, 70.002)),
                                               module, W::DRUM0_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.75, 70.002)),
                                               module, W::DRUM1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.501, 70.002)),
                                               module, W::DRUM2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.249, 70.002)),
                                               module, W::DRUM3_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.0, 70.002)), module,
                                               W::DRUM4_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.999, 112.999)),
                                               module, W::DRUM5_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.75, 112.999)),
                                               module, W::DRUM6_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.501, 112.999)),
                                               module, W::DRUM7_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.249, 112.999)),
                                               module, W::DRUM8_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.0, 112.999)),
                                               module, W::DRUM9_OUTPUT));
  }
};
