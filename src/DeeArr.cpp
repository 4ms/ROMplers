#include "DeeArrSamples.hpp"
#include "plugin.hpp"

struct SpeedQuantity : ParamQuantity {
  std::string getDisplayValueString() override {
    float v = getValue();
    float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
    return string::f("%.3gx", display);
  }
};

struct DeeArr : Module {
  enum ParamId {
    SPEED_PARAM,
    LENGTH_PARAM,
    MAINVOL_PARAM,
    KICKPUSH_PARAM,
    SNAREPUSH_PARAM,
    HATPUSH_PARAM,
    RIMPUSH_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SPEEDCVIN_INPUT,
    LENGTHCVIN_INPUT,
    KICKTRIGIN_INPUT,
    SNARETRIGIN_INPUT,
    HATTRIGIN_INPUT,
    RIMTRIGIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    KICKOUT_OUTPUT,
    SNAREOUT_OUTPUT,
    HATOUT_OUTPUT,
    RIMOUT_OUTPUT,
    SUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId { KICK_LIGHT, SNARE_LIGHT, HAT_LIGHT, RIM_LIGHT, LIGHTS_LEN };

  struct Voice {
    bool lastInputTrigger = false;
    bool lastButtonTrigger = false;
    float samplePos = 0.f;
    bool playing = false;
    int outputId = 0;
    int lightId = -1;

    // Pre-decoded float samples
    std::vector<float> decodedSample;

    // Output zero flag to avoid redundant zero output writes
    bool outputZeroSet = true;

    void loadSample(const unsigned char *data, int lengthBytes) {
      int sampleCount = lengthBytes / 2;
      decodedSample.resize(sampleCount);
      for (int i = 0; i < sampleCount; ++i) {
        int16_t s = (int16_t)(data[2 * i] | (data[2 * i + 1] << 8));
        decodedSample[i] = s / 32768.f;
      }
      samplePos = 0.f;
      playing = false;
      outputZeroSet = true;
    }
  };

  const float SPEED_LOW = 0.05f;
  const float SPEED_HIGH = 2.0f;
  const float LENGTH_MIN = 0.1f;
  const float LENGTH_MAX = 1.0f;

  Voice kickVoice, snareVoice, hatVoice, rimVoice;

  DeeArr() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
    configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
    configParam(MAINVOL_PARAM, 0.f, 1.f, 0.5f, "Main Volume", "%", 0.f, 100.f);
    configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
    configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
    configSwitch(HATPUSH_PARAM, 0.f, 1.f, 0.f, "Hat Trig", {"Off", "On"});
    configSwitch(RIMPUSH_PARAM, 0.f, 1.f, 0.f, "Rim Trig", {"Off", "On"});

    configInput(SPEEDCVIN_INPUT, "Speed CV");
    configInput(LENGTHCVIN_INPUT, "Length CV");
    configInput(KICKTRIGIN_INPUT, "Kick Trig");
    configInput(SNARETRIGIN_INPUT, "Snare Trig");
    configInput(HATTRIGIN_INPUT, "Hat Trig");
    configInput(RIMTRIGIN_INPUT, "Rim Trig");

    configOutput(KICKOUT_OUTPUT, "Kick");
    configOutput(SNAREOUT_OUTPUT, "Snare");
    configOutput(HATOUT_OUTPUT, "Hat");
    configOutput(RIMOUT_OUTPUT, "Rim");
    configOutput(SUM_OUTPUT, "Sum");

    // Preload samples once, decoding into float vectors
    kickVoice = createVoice(DAKick, sizeof(DAKick), KICKOUT_OUTPUT, KICK_LIGHT);
    snareVoice =
        createVoice(DASnare, sizeof(DASnare), SNAREOUT_OUTPUT, SNARE_LIGHT);
    hatVoice = createVoice(DAHat, sizeof(DAHat), HATOUT_OUTPUT, HAT_LIGHT);
    rimVoice = createVoice(DARim, sizeof(DARim), RIMOUT_OUTPUT, RIM_LIGHT);
  }

  Voice createVoice(const unsigned char *data, size_t size, int outputId,
                    int lightId) {
    Voice v;
    v.outputId = outputId;
    v.lightId = lightId;
    v.loadSample(data, (int)size);
    return v;
  }

  void process(const ProcessArgs &args) override {
    // --- Precalculate speed with CV ---
    float knobSpeedV = (params[SPEED_PARAM].getValue() + 1.f) * 2.5f;
    float speedCVV = 0.f;
    if (inputs[SPEEDCVIN_INPUT].isConnected())
      speedCVV =
          std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f;
    float normSpeed = (std::clamp(knobSpeedV + speedCVV, 0.f, 5.f) / 5.f);
    float speed = SPEED_LOW + normSpeed * (SPEED_HIGH - SPEED_LOW);

    // --- Precalculate length with CV ---
    float knobLengthV = params[LENGTH_PARAM].getValue() * 5.f;
    float lengthCVV = 0.f;
    if (inputs[LENGTHCVIN_INPUT].isConnected())
      lengthCVV =
          std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f;
    float normLength = std::clamp(knobLengthV + lengthCVV, 0.f, 5.f) / 5.f;
    float lengthRatio = LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);

    // --- Process each voice ---
    processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, hatVoice, HATTRIGIN_INPUT, HATPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, rimVoice, RIMTRIGIN_INPUT, RIMPUSH_PARAM, speed,
                 lengthRatio);

    // --- Sum output ---
    float mainVol = params[MAINVOL_PARAM].getValue();
    float busSum = 0.f;
    Voice *allVoices[] = {&kickVoice, &snareVoice, &hatVoice, &rimVoice};
    for (Voice *v : allVoices) {
      if (!outputs[v->outputId].isConnected()) {
        busSum += outputs[v->outputId].getVoltage();
      }
    }
    outputs[SUM_OUTPUT].setVoltage(
        std::clamp(busSum * mainVol * 4.f, -10.f, 10.f));
  }

  void processVoice(const ProcessArgs &args, Voice &voice, int trigInputId,
                    int pushParamId, float speed, float lengthRatio) {
    const bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
    const bool buttonTrigger = params[pushParamId].getValue() > 0.5f;

    const bool inputRising = inputTrigger && !voice.lastInputTrigger;
    const bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;

    voice.lastInputTrigger = inputTrigger;
    voice.lastButtonTrigger = buttonTrigger;

    bool fired = false;

    if (inputRising || buttonRising) {
      voice.playing = true;
      voice.samplePos = 0.f;
      fired = true;
      if (voice.lightId >= 0)
        lights[voice.lightId].setBrightness(1.f);
      voice.outputZeroSet = false;
    }

    if (voice.playing) {
      int maxSamplesToPlay = (int)(voice.decodedSample.size() * lengthRatio);
      int idx = (int)voice.samplePos;

      if (idx < maxSamplesToPlay) {
        float sample = voice.decodedSample[idx];
        outputs[voice.outputId].setVoltage(sample * 5.f);
        voice.samplePos += speed;
      } else {
        voice.playing = false;
        outputs[voice.outputId].setVoltage(0.f);
        voice.outputZeroSet = true;
      }
    } else {
      outputs[voice.outputId].setVoltage(0.f);
    }

    // Light handling
    if (voice.lightId >= 0) {
      if (fired)
        lights[voice.lightId].setBrightness(1.f);
      else
        lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
    }
  }
};

struct DeeArrWidget : ModuleWidget {
  DeeArrWidget(DeeArr *module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/panels/DeeArr.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Davies1900hBlack>(
        mm2px(Vec(25.4, 19.001)), module, DeeArr::SPEED_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(
        mm2px(Vec(9.751, 29.2)), module, DeeArr::LENGTH_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(
        mm2px(Vec(41.25, 29.2)), module, DeeArr::MAINVOL_PARAM));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67.0)), module,
                                           DeeArr::KICKPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 67.0)), module, DeeArr::KICK_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82.0)), module,
                                           DeeArr::SNAREPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 82.0)), module, DeeArr::SNARE_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97.0)), module,
                                           DeeArr::HATPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 97.0)), module, DeeArr::HAT_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module,
                                           DeeArr::RIMPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 112.0)), module, DeeArr::RIM_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 36.499)), module,
                                             DeeArr::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.751, 47.001)), module,
                                             DeeArr::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module,
                                             DeeArr::KICKTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module,
                                             DeeArr::SNARETRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module,
                                             DeeArr::HATTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module,
                                             DeeArr::RIMTRIGIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(41.25, 47.001)),
                                               module, DeeArr::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module,
                                               DeeArr::KICKOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module,
                                               DeeArr::SNAREOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module,
                                               DeeArr::HATOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)),
                                               module, DeeArr::RIMOUT_OUTPUT));
  }
};

Model *modelDeeArr = createModel<DeeArr, DeeArrWidget>("DeeArr");
