#include "SinSahnixSamples.hpp"
#include "dsp_utils.hh"
#include "plugin.hpp"

struct SpeedQuantity : ParamQuantity {
  std::string getDisplayValueString() override {
    float v = getValue();
    float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
    return string::f("%.3gx", display);
  }
};

struct SinSahnix : Module {
  enum ParamId {
    SPEED_PARAM,
    LENGTH_PARAM,
    MAINVOL_PARAM,
    KICKPUSH_PARAM,
    SNAREPUSH_PARAM,
    TOMLPUSH_PARAM,
    TOMMPUSH_PARAM,
    TOMHPUSH_PARAM,
    CYMPUSH_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SPEEDCVIN_INPUT,
    LENGTHCVIN_INPUT,
    KICKTRIGIN_INPUT,
    SNARETRIGIN_INPUT,
    TOMLTRIGIN_INPUT,
    TOMMTRIGIN_INPUT,
    TOMHTRIGIN_INPUT,
    CYMTRIGIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    KICKOUT_OUTPUT,
    SNAREOUT_OUTPUT,
    TOMLOUT_OUTPUT,
    TOMMOUT_OUTPUT,
    TOMHOUT_OUTPUT,
    CYMOUT_OUTPUT,
    SUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    KICK_LIGHT,
    SNARE_LIGHT,
    TOML_LIGHT,
    TOMM_LIGHT,
    TOMH_LIGHT,
    CYM_LIGHT,
    LIGHTS_LEN
  };

  struct Voice {
    bool lastInputTrigger = false;
    bool lastButtonTrigger = false;

    float samplePos = 0.f;
    bool playing = false;
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
  Voice tomMedVoice;
  Voice tomHiVoice;
  Voice cymbalVoice;

  const float LENGTH_MIN = 0.1f;
  const float LENGTH_MAX = 1.0f;

  SinSahnix() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
    configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
    configParam(MAINVOL_PARAM, 0.f, 1.f, 0.5f, "Main Volume", "%", 0.f, 100.f);

    configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
    configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
    configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
    configSwitch(TOMMPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Mid Trig", {"Off", "On"});
    configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
    configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});

    configInput(SPEEDCVIN_INPUT, "Speed CV");
    configInput(LENGTHCVIN_INPUT, "Length CV");
    configInput(KICKTRIGIN_INPUT, "Kick Trig");
    configInput(SNARETRIGIN_INPUT, "Snare Trig");
    configInput(TOMLTRIGIN_INPUT, "Tom Lo Trig");
    configInput(TOMMTRIGIN_INPUT, "Tom Mid Trig");
    configInput(TOMHTRIGIN_INPUT, "Tom Hi Trig");
    configInput(CYMTRIGIN_INPUT, "Cymbal Trig");

    configOutput(KICKOUT_OUTPUT, "Kick");
    configOutput(SNAREOUT_OUTPUT, "Snare");
    configOutput(TOMLOUT_OUTPUT, "Tom Lo");
    configOutput(TOMMOUT_OUTPUT, "Tom Mid");
    configOutput(TOMHOUT_OUTPUT, "Tom Hi");
    configOutput(CYMOUT_OUTPUT, "Cymbal");
    configOutput(SUM_OUTPUT, "Sum");

    kickVoice.rawData = SSKick;
    kickVoice.sampleLength = sizeof(SSKick) / 2;
    kickVoice.outputId = KICKOUT_OUTPUT;
    kickVoice.lightId = KICK_LIGHT;

    snareVoice.rawData = SSSnare;
    snareVoice.sampleLength = sizeof(SSSnare) / 2;
    snareVoice.outputId = SNAREOUT_OUTPUT;
    snareVoice.lightId = SNARE_LIGHT;

    tomLoVoice.rawData = SSTomL;
    tomLoVoice.sampleLength = sizeof(SSTomL) / 2;
    tomLoVoice.outputId = TOMLOUT_OUTPUT;
    tomLoVoice.lightId = TOML_LIGHT;

    tomMedVoice.rawData = SSTomM;
    tomMedVoice.sampleLength = sizeof(SSTomM) / 2;
    tomMedVoice.outputId = TOMMOUT_OUTPUT;
    tomMedVoice.lightId = TOMM_LIGHT;

    tomHiVoice.rawData = SSTomH;
    tomHiVoice.sampleLength = sizeof(SSTomH) / 2;
    tomHiVoice.outputId = TOMHOUT_OUTPUT;
    tomHiVoice.lightId = TOMH_LIGHT;

    cymbalVoice.rawData = SSCymbal;
    cymbalVoice.sampleLength = sizeof(SSCymbal) / 2;
    cymbalVoice.outputId = CYMOUT_OUTPUT;
    cymbalVoice.lightId = CYM_LIGHT;
  }

  void process(const ProcessArgs &args) override {
    // Knob (-1..1) rescaled to -5..5V offset; CV added and clamped to ±5V
    const float knobPitchOffset = params[SPEED_PARAM].getValue() * 5.f;
    const float pitchCV = inputs[SPEEDCVIN_INPUT].isConnected()
                              ? inputs[SPEEDCVIN_INPUT].getVoltage()
                              : 0.f;
    float pitchMod = rescale(std::clamp(knobPitchOffset + pitchCV, -5.f, 5.f), -5.f, 5.f, -1.f, 1.f);

    float normalizedPitch = (pitchMod + 1.f) * 0.5f;
    float pitchRatio = calcPitchRatio(normalizedPitch);

    const float lengthCV = inputs[LENGTHCVIN_INPUT].isConnected()
                               ? inputs[LENGTHCVIN_INPUT].getVoltage()
                               : 0.f;
    const float normLength = calcDecayMod(params[LENGTH_PARAM].getValue(), lengthCV);
    const float lengthRatio = LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);

    processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, tomLoVoice, TOMLTRIGIN_INPUT, TOMLPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, tomMedVoice, TOMMTRIGIN_INPUT, TOMMPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, tomHiVoice, TOMHTRIGIN_INPUT, TOMHPUSH_PARAM, pitchRatio,
                 lengthRatio);
    processVoice(args, cymbalVoice, CYMTRIGIN_INPUT, CYMPUSH_PARAM, pitchRatio,
                 lengthRatio);

    // --- Sum output ---
    float mainVol = params[MAINVOL_PARAM].getValue();
    float busSum = 0.f;
    Voice *allVoices[] = {&kickVoice,   &snareVoice, &tomLoVoice,
                          &tomMedVoice, &tomHiVoice, &cymbalVoice};
    for (Voice *v : allVoices) {
      if (!outputs[v->outputId].isConnected()) {
        busSum += outputs[v->outputId].getVoltage() / 2.f;
      }
    }
    outputs[SUM_OUTPUT].setVoltage(
        std::clamp(busSum / 6.f * mainVol * 6.25f, -10.f, 10.f));
  }

  void processVoice(const ProcessArgs &args, Voice &voice, int trigInputId,
                    int pushParamId, float pitchRatio, float lengthRatio) {
    const bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
    const bool buttonTrigger = params[pushParamId].getValue() > 0.5f;

    const bool inputRising = inputTrigger && !voice.lastInputTrigger;
    const bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;

    voice.lastInputTrigger = inputTrigger;
    voice.lastButtonTrigger = buttonTrigger;

    bool fired = inputRising || buttonRising;

    if (fired) {
      voice.playing = true;
      voice.samplePos = 0.f;
      fired = true;
    }

    if (!voice.playing) {
      outputs[voice.outputId].setVoltage(0.f);
      if (voice.lightId >= 0)
        lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
      return;
    }

    const int maxSamplesToPlay = (int)(voice.sampleLength * lengthRatio);
    int idx = (int)voice.samplePos;

    if (idx < maxSamplesToPlay) {
      const int16_t sampleInt = voice.readSample16(idx);
      const float sample = (float)sampleInt / 32768.f;
      outputs[voice.outputId].setVoltage(sample * 10.f);
      voice.samplePos += pitchRatio;
    } else {
      voice.playing = false;
      outputs[voice.outputId].setVoltage(0.f);
    }

    if (voice.lightId >= 0) {
      if (fired)
        lights[voice.lightId].setBrightness(1.f);
      else
        lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
    }
  }
};

struct SinSahnixWidget : ModuleWidget {
  SinSahnixWidget(SinSahnix *module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/panels/SinSahnix.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module,
                                          SinSahnix::LENGTH_PARAM));
    addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module,
                                          SinSahnix::SPEED_PARAM));

    addParam(createParamCentered<Knob9mm>(mm2px(Vec(44.2, 12.45)), module,
                                          SinSahnix::MAINVOL_PARAM));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 37)), module,
                                           SinSahnix::KICKPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 37)), module, SinSahnix::KICK_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 52)), module,
                                           SinSahnix::SNAREPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 52)), module, SinSahnix::SNARE_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67)), module,
                                           SinSahnix::TOMLPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 67)), module, SinSahnix::TOML_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82)), module,
                                           SinSahnix::TOMMPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 82)), module, SinSahnix::TOMM_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97)), module,
                                           SinSahnix::TOMHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 97)), module, SinSahnix::TOMH_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112)), module,
                                           SinSahnix::CYMPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 112)), module, SinSahnix::CYM_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module,
                                             SinSahnix::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module,
                                             SinSahnix::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module,
                                             SinSahnix::KICKTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 52.0)), module,
                                             SinSahnix::SNARETRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module,
                                             SinSahnix::TOMLTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module,
                                             SinSahnix::TOMMTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module,
                                             SinSahnix::TOMHTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module,
                                             SinSahnix::CYMTRIGIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module,
                                               SinSahnix::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module,
                                               SinSahnix::KICKOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 52.0)), module,
                                               SinSahnix::SNAREOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module,
                                               SinSahnix::TOMLOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module,
                                               SinSahnix::TOMMOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module,
                                               SinSahnix::TOMHOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(43.998, 112.0)), module, SinSahnix::CYMOUT_OUTPUT));
  }
};

Model *modelSinSahnix = createModel<SinSahnix, SinSahnixWidget>("SinSahnix");
