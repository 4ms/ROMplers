#include "KayOneSamples.hpp"
#include "plugin.hpp"

struct SpeedQuantity : ParamQuantity {
  std::string getDisplayValueString() override {
    float v = getValue();
    float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
    return string::f("%.3gx", display);
  }
};

struct KayOne : Module {
  enum ParamId {
    SPEED_PARAM,
    LENGTH_PARAM,
    MAINVOL_PARAM,
    KICKPUSH_PARAM,
    SNAREPUSH_PARAM,
    TOMLPUSH_PARAM,
    TOMHPUSH_PARAM,
    CLPUSH_PARAM,
    OHPUSH_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SPEEDCVIN_INPUT,
    LENGTHCVIN_INPUT,
    KICKTRIGIN_INPUT,
    SNARETRIGIN_INPUT,
    TOMLTRIG_INPUT,
    TOMHTRIG_INPUT,
    CLTRIG_INPUT,
    OHTRIG_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    KICKOUT_OUTPUT,
    SNAREOUT_OUTPUT,
    TOMLOUT_OUTPUT,
    TOMHOUT_OUTPUT,
    CLOUT_OUTPUT,
    OHOUT_OUTPUT,
    SUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    KICK_LIGHT,
    SNARE_LIGHT,
    TOML_LIGHT,
    TOMH_LIGHT,
    CL_LIGHT,
    OH_LIGHT,
    LIGHTS_LEN
  };

  struct Voice {
    bool lastInputTrigger = false;
    bool lastButtonTrigger = false;

    float samplePos = 0.f;
    bool playing = false;
    bool lastTriggerState = false;
    const unsigned char *rawData = nullptr;
    int sampleLength = 0;
    int outputId = 0;
    int lightId = -1;

    int16_t readSample16(int index) {
      return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
    }
  };

  Voice kickVoice;
  Voice snareVoice;
  Voice tomLoVoice;
  Voice tomHiVoice;
  Voice closedHatVoice;
  Voice openHatVoice;

  const float MIN_PLAYBACK_SPEED = 0.05f;
  const float MAX_PLAYBACK_SPEED = 1.0f;
  const float LENGTH_MIN = 0.1f;
  const float LENGTH_MAX = 1.0f;

  KayOne() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
    configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
    configParam(MAINVOL_PARAM, 0.f, 1.f, 0.5f, "Main Volume", "%", 0.f, 100.f);

    configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
    configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
    configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
    configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
    configSwitch(CLPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
    configSwitch(OHPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});

    configInput(SPEEDCVIN_INPUT, "Speed CV");
    configInput(LENGTHCVIN_INPUT, "Length CV");
    configInput(KICKTRIGIN_INPUT, "Kick Trig");
    configInput(SNARETRIGIN_INPUT, "Snare Trig");
    configInput(TOMLTRIG_INPUT, "Tom Lo Trig");
    configInput(TOMHTRIG_INPUT, "Tom Hi Trig");
    configInput(CLTRIG_INPUT, "Closed Hat Trig");
    configInput(OHTRIG_INPUT, "Open Hat Trig");

    configOutput(KICKOUT_OUTPUT, "Kick");
    configOutput(SNAREOUT_OUTPUT, "Snare");
    configOutput(TOMLOUT_OUTPUT, "Tom Lo");
    configOutput(TOMHOUT_OUTPUT, "Tom Hi");
    configOutput(CLOUT_OUTPUT, "Closed Hat");
    configOutput(OHOUT_OUTPUT, "Open Hat");
    configOutput(SUM_OUTPUT, "Sum");

    kickVoice.rawData = SKKick;
    kickVoice.sampleLength = sizeof(SKKick) / 2;
    kickVoice.outputId = KICKOUT_OUTPUT;
    kickVoice.lightId = KICK_LIGHT;

    snareVoice.rawData = SKSnare;
    snareVoice.sampleLength = sizeof(SKSnare) / 2;
    snareVoice.outputId = SNAREOUT_OUTPUT;
    snareVoice.lightId = SNARE_LIGHT;

    tomLoVoice.rawData = SKTomLo;
    tomLoVoice.sampleLength = sizeof(SKTomLo) / 2;
    tomLoVoice.outputId = TOMLOUT_OUTPUT;
    tomLoVoice.lightId = TOML_LIGHT;

    tomHiVoice.rawData = SKTomHi;
    tomHiVoice.sampleLength = sizeof(SKTomHi) / 2;
    tomHiVoice.outputId = TOMHOUT_OUTPUT;
    tomHiVoice.lightId = TOMH_LIGHT;

    closedHatVoice.rawData = SKClosedHat;
    closedHatVoice.sampleLength = sizeof(SKClosedHat) / 2;
    closedHatVoice.outputId = CLOUT_OUTPUT;
    closedHatVoice.lightId = CL_LIGHT;

    openHatVoice.rawData = SKOpenHat;
    openHatVoice.sampleLength = sizeof(SKOpenHat) / 2;
    openHatVoice.outputId = OHOUT_OUTPUT;
    openHatVoice.lightId = OH_LIGHT;
  }

  void process(const ProcessArgs &args) override {
    static const float lengthCVScale = 0.1f;

    // Knob (-1..1) rescaled to -5..5V offset; CV added and clamped to ±5V
    const float knobPitchOffset = params[SPEED_PARAM].getValue() * 5.f;
    const float pitchCV = inputs[SPEEDCVIN_INPUT].isConnected()
                              ? inputs[SPEEDCVIN_INPUT].getVoltage()
                              : 0.f;
    float pitchMod = rescale(std::clamp(knobPitchOffset + pitchCV, -5.f, 5.f), -5.f, 5.f, -1.f, 1.f);

    float normalizedPitch = (pitchMod + 1.f) * 0.5f;
    float pitchRatio =
        MIN_PLAYBACK_SPEED +
        normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);

    float knobLength = params[LENGTH_PARAM].getValue();
    float normLength = std::clamp(
        knobLength + inputs[LENGTHCVIN_INPUT].getVoltage() * lengthCVScale,
        0.1f, 1.0f);
    float lengthRatio =
        LENGTH_MIN +
        (normLength - 0.1f) * ((LENGTH_MAX - LENGTH_MIN) / (1.0f - 0.1f));

    // Process all voices
    processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, tomLoVoice, TOMLTRIG_INPUT, TOMLPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, tomHiVoice, TOMHTRIG_INPUT, TOMHPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, closedHatVoice, CLTRIG_INPUT, CLPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, openHatVoice, OHTRIG_INPUT, OHPUSH_PARAM, pitchRatio,
                 lengthRatio);

    // --- Sum output ---
    float mainVol = params[MAINVOL_PARAM].getValue();
    float busSum = 0.f;
    Voice *allVoices[] = {&kickVoice,  &snareVoice,     &tomLoVoice,
                          &tomHiVoice, &closedHatVoice, &openHatVoice};
    for (Voice *v : allVoices) {
      if (!outputs[v->outputId].isConnected()) {
        busSum += outputs[v->outputId].getVoltage();
      }
    }
    outputs[SUM_OUTPUT].setVoltage(std::clamp(busSum / 6.f * mainVol * (10.f / 3.f), -10.f, 10.f));
  }

  void processVoice(const ProcessArgs &args, Voice &voice, int trigInputId,
                    int pushParamId, float pitchRatio, float lengthRatio) {
    static constexpr float SAMPLE_SCALE = 5.0f / 32768.0f;

    bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
    bool buttonTrigger = params[pushParamId].getValue() > 0.5f;

    bool inputRising = inputTrigger && !voice.lastInputTrigger;
    bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;

    bool triggered = inputRising || buttonRising;
    bool fired = triggered;

    if (triggered) {
      voice.playing = true;
      voice.samplePos = 0.f;
      if (voice.lightId >= 0)
        lights[voice.lightId].setBrightness(1.0f);
    }

    voice.lastInputTrigger = inputTrigger;
    voice.lastButtonTrigger = buttonTrigger;

    int maxSamplesToPlay = static_cast<int>(voice.sampleLength * lengthRatio);
    float sample = 0.f;

    if (voice.playing) {
      int idx = static_cast<int>(voice.samplePos);
      if (idx < maxSamplesToPlay) {
        int16_t sampleInt = voice.readSample16(idx);
        sample = sampleInt * SAMPLE_SCALE;
        voice.samplePos += pitchRatio;
      } else {
        voice.playing = false;
      }
    }

    if (outputs[voice.outputId].getVoltage() != sample) {
      outputs[voice.outputId].setVoltage(sample * 2.f);
    }

    if (voice.lightId >= 0) {
      if (fired)
        lights[voice.lightId].setBrightness(1.0f);
      else
        lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
    }
  }
};

struct KayOneWidget : ModuleWidget {
  KayOneWidget(KayOne *module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/panels/KayOne.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module,
                                          KayOne::LENGTH_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module,
                                          KayOne::SPEED_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(44.2, 12.45)), module,
                                          KayOne::MAINVOL_PARAM));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 37.0)), module,
                                           KayOne::KICKPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 37.0)), module, KayOne::KICK_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 52.0)), module,
                                           KayOne::SNAREPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 52.0)), module, KayOne::SNARE_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67.0)), module,
                                           KayOne::TOMLPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 67.0)), module, KayOne::TOML_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82.0)), module,
                                           KayOne::TOMHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 82.0)), module, KayOne::TOMH_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97.0)), module,
                                           KayOne::CLPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 97.0)), module, KayOne::CL_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module,
                                           KayOne::OHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 112.0)), module, KayOne::OH_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module,
                                             KayOne::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module,
                                             KayOne::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module,
                                             KayOne::KICKTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 52.0)), module,
                                             KayOne::SNARETRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module,
                                             KayOne::TOMLTRIG_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module,
                                             KayOne::TOMHTRIG_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module,
                                             KayOne::CLTRIG_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module,
                                             KayOne::OHTRIG_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module,
                                               KayOne::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module,
                                               KayOne::KICKOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 52.0)), module,
                                               KayOne::SNAREOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module,
                                               KayOne::TOMLOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module,
                                               KayOne::TOMHOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module,
                                               KayOne::CLOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)),
                                               module, KayOne::OHOUT_OUTPUT));
  }
};

Model *modelKayOne = createModel<KayOne, KayOneWidget>("KayOne");
