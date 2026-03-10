#include "SehvenTooSamples.hpp"
#include "plugin.hpp"

struct SpeedQuantity : ParamQuantity {
  std::string getDisplayValueString() override {
    float v = getValue();
    float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
    return string::f("%.3gx", display);
  }
};

struct SehvenToo : Module {
  enum ParamId {
    SPEED_PARAM,
    LENGTH_PARAM,
    MAINVOL_PARAM,
    CONGALPUSH_PARAM,
    CONGAHPUSH_PARAM,
    CONGAHMPUSH_PARAM,
    BONGOLPUSH_PARAM,
    BONGOHPUSH_PARAM,
    TIMBALELPUSH_PARAM,
    TIMBALEHPUSH_PARAM,
    AGOGOLPUSH_PARAM,
    AGOGOHPUSH_PARAM,
    MARACAPUSH_PARAM,
    CABASAPUSH_PARAM,
    WHISTLEPUSH_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SPEEDCVIN_INPUT,
    LENGTHCVIN_INPUT,
    CONGALTRIGIN_INPUT,
    CONGAHTRIGIN_INPUT,
    CONGAHMTRIGIN_INPUT,
    BONGOLTRIGIN_INPUT,
    BONGOHTRIGIN_INPUT,
    TIMBALELTRIGIN_INPUT,
    TIMBALEHTRIGIN_INPUT,
    AGOGOLTRIGIN_INPUT,
    AGOGOHTRIGIN_INPUT,
    MARACATRIGIN_INPUT,
    CABASATRIGIN_INPUT,
    WHISTLETRIGIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    CONGALOUT_OUTPUT,
    CONGAHOUT_OUTPUT,
    CONGAHMOUT_OUTPUT,
    BONGOLOUT_OUTPUT,
    BONGOHOUT_OUTPUT,
    TIMBALELOUT_OUTPUT,
    TIMBALEHOUT_OUTPUT,
    AGOGOLOUT_OUTPUT,
    AGOGOHOUT_OUTPUT,
    MARACAOUT_OUTPUT,
    CABASAOUT_OUTPUT,
    WHISTLEOUT_OUTPUT,
    SUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    CONGAL_LIGHT,
    CONGAH_LIGHT,
    CONGAHM_LIGHT,
    BONGOL_LIGHT,
    BONGOH_LIGHT,
    TIMBALEL_LIGHT,
    TIMBALEH_LIGHT,
    AGOGOL_LIGHT,
    AGOGOH_LIGHT,
    MARACA_LIGHT,
    CABASA_LIGHT,
    WHISTLE_LIGHT,
    LIGHTS_LEN
  };

  struct Voice {
    bool lastInputTrigger = false, lastButtonTrigger = false;
    float samplePos = 0.f;
    bool playing = false;
    const unsigned char *rawData = nullptr;
    int sampleLength = 0;
    int outputId = 0, lightId = -1;
    int16_t readSample16(int index) {
      return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
    }
  };

  Voice congaLVoice, congaHVoice, congaHMVoice;
  Voice bongoLVoice, bongoHVoice;
  Voice timbaleLVoice, timbaleHVoice;
  Voice agogoLVoice, agogoHVoice;
  Voice maracaVoice, cabasaVoice, whistleVoice;

  Voice createVoice(const unsigned char *data, size_t size, int oId, int lId) {
    Voice v;
    v.rawData = data;
    v.sampleLength = size / 2;
    v.outputId = oId;
    v.lightId = lId;
    return v;
  }

  const float SPEED_MIN = 0.05f;
  const float SPEED_MAX = 2.0f;
  const float LENGTH_MIN = 0.1f;
  const float LENGTH_MAX = 1.0f;

  SehvenToo() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
    configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
    configParam(MAINVOL_PARAM, 0.f, 1.f, 0.5f, "Main Volume", "%", 0.f, 100.f);
    configSwitch(CONGALPUSH_PARAM, 0.f, 1.f, 0.f, "Conga Lo Trig",
                 {"Off", "On"});
    configSwitch(CONGAHPUSH_PARAM, 0.f, 1.f, 0.f, "Conga Hi Trig",
                 {"Off", "On"});
    configSwitch(CONGAHMPUSH_PARAM, 0.f, 1.f, 0.f, "Conga Hi Mute Trig",
                 {"Off", "On"});
    configSwitch(BONGOLPUSH_PARAM, 0.f, 1.f, 0.f, "Bongo Lo Trig",
                 {"Off", "On"});
    configSwitch(BONGOHPUSH_PARAM, 0.f, 1.f, 0.f, "Bongo Hi Trig",
                 {"Off", "On"});
    configSwitch(TIMBALELPUSH_PARAM, 0.f, 1.f, 0.f, "Timbale Lo Trig",
                 {"Off", "On"});
    configSwitch(TIMBALEHPUSH_PARAM, 0.f, 1.f, 0.f, "Timbale Hi Trig",
                 {"Off", "On"});
    configSwitch(AGOGOLPUSH_PARAM, 0.f, 1.f, 0.f, "Agogo Lo Trig",
                 {"Off", "On"});
    configSwitch(AGOGOHPUSH_PARAM, 0.f, 1.f, 0.f, "Agogo Hi Trig",
                 {"Off", "On"});
    configSwitch(MARACAPUSH_PARAM, 0.f, 1.f, 0.f, "Maraca Trig", {"Off", "On"});
    configSwitch(CABASAPUSH_PARAM, 0.f, 1.f, 0.f, "Cabasa Trig", {"Off", "On"});
    configSwitch(WHISTLEPUSH_PARAM, 0.f, 1.f, 0.f, "Whistle Trig",
                 {"Off", "On"});
    configInput(SPEEDCVIN_INPUT, "Speed CV");
    configInput(LENGTHCVIN_INPUT, "Length CV");
    configInput(CONGALTRIGIN_INPUT, "Conga Lo Trig");
    configInput(CONGAHTRIGIN_INPUT, "Conga Hi Trig");
    configInput(CONGAHMTRIGIN_INPUT, "Conga Hi Mute Trig");
    configInput(BONGOLTRIGIN_INPUT, "Bongo Lo Trig");
    configInput(BONGOHTRIGIN_INPUT, "Bongo Hi Trig");
    configInput(TIMBALELTRIGIN_INPUT, "Timbale Lo Trig");
    configInput(TIMBALEHTRIGIN_INPUT, "Timbale Hi Trig");
    configInput(AGOGOLTRIGIN_INPUT, "Agogo Lo Trig");
    configInput(AGOGOHTRIGIN_INPUT, "Agogo Hi Trig");
    configInput(MARACATRIGIN_INPUT, "Maraca Trig");
    configInput(CABASATRIGIN_INPUT, "Cabasa Trig");
    configInput(WHISTLETRIGIN_INPUT, "Whistle Trig");
    configOutput(CONGALOUT_OUTPUT, "Conga Lo");
    configOutput(CONGAHOUT_OUTPUT, "Conga Hi");
    configOutput(CONGAHMOUT_OUTPUT, "Conga Hi Mute");
    configOutput(BONGOLOUT_OUTPUT, "Bongo Lo");
    configOutput(BONGOHOUT_OUTPUT, "Bongo Hi");
    configOutput(TIMBALELOUT_OUTPUT, "Timbale Lo");
    configOutput(TIMBALEHOUT_OUTPUT, "Timbale Hi");
    configOutput(AGOGOLOUT_OUTPUT, "Agogo Lo");
    configOutput(AGOGOHOUT_OUTPUT, "Agogo Hi");
    configOutput(MARACAOUT_OUTPUT, "Maraca");
    configOutput(CABASAOUT_OUTPUT, "Cabasa");
    configOutput(WHISTLEOUT_OUTPUT, "Whistle");
    configOutput(SUM_OUTPUT, "Sum");

    congaLVoice =
        createVoice(STCongaL, sizeof(STCongaL), CONGALOUT_OUTPUT, CONGAL_LIGHT);
    congaHVoice = createVoice(STCongaHOpen, sizeof(STCongaHOpen),
                              CONGAHOUT_OUTPUT, CONGAH_LIGHT);
    congaHMVoice = createVoice(STCongaHMute, sizeof(STCongaHMute),
                               CONGAHMOUT_OUTPUT, CONGAHM_LIGHT);
    bongoLVoice =
        createVoice(STBongoL, sizeof(STBongoL), BONGOLOUT_OUTPUT, BONGOL_LIGHT);
    bongoHVoice =
        createVoice(STBongoH, sizeof(STBongoH), BONGOHOUT_OUTPUT, BONGOH_LIGHT);
    timbaleLVoice = createVoice(STTimbaleL, sizeof(STTimbaleL),
                                TIMBALELOUT_OUTPUT, TIMBALEL_LIGHT);
    timbaleHVoice = createVoice(STTimbaleH, sizeof(STTimbaleH),
                                TIMBALEHOUT_OUTPUT, TIMBALEH_LIGHT);
    agogoLVoice =
        createVoice(STAgogoL, sizeof(STAgogoL), AGOGOLOUT_OUTPUT, AGOGOL_LIGHT);
    agogoHVoice =
        createVoice(STAgogoH, sizeof(STAgogoH), AGOGOHOUT_OUTPUT, AGOGOH_LIGHT);
    maracaVoice = createVoice(STMaracas, sizeof(STMaracas), MARACAOUT_OUTPUT,
                              MARACA_LIGHT);
    cabasaVoice =
        createVoice(STCabasa, sizeof(STCabasa), CABASAOUT_OUTPUT, CABASA_LIGHT);
    whistleVoice = createVoice(STWhistle, sizeof(STWhistle), WHISTLEOUT_OUTPUT,
                               WHISTLE_LIGHT);
  }

  void processVoice(const ProcessArgs &args, Voice &v, int trigInId,
                    int pushParamId, float speed, float lengthRatio) {

    bool inTrig = inputs[trigInId].getVoltage() > 1.f;
    bool btnTrig = params[pushParamId].getValue() > 0.5f;
    bool inputRising = inTrig && !v.lastInputTrigger;
    bool buttonRising = btnTrig && !v.lastButtonTrigger;

    bool fired = inputRising || buttonRising;

    if (fired) {
      v.playing = true;
      v.samplePos = 0.f;
    }

    v.lastInputTrigger = inTrig;
    v.lastButtonTrigger = btnTrig;

    int maxSamples = (int)(v.sampleLength * lengthRatio);

    if (v.playing) {
      int idx = int(v.samplePos);
      if (idx < maxSamples) {
        int16_t s = v.readSample16(idx);
        float out = (float)s / 32768.f;
        outputs[v.outputId].setVoltage(out * 10.f);
        v.samplePos += speed;
      } else {
        v.playing = false;
        outputs[v.outputId].setVoltage(0.f);
      }
    } else {
      outputs[v.outputId].setVoltage(0.f);
    }

    if (v.lightId >= 0) {
      if (fired)
        lights[v.lightId].setBrightness(1.f);
      else
        lights[v.lightId].setBrightnessSmooth(0.f, args.sampleTime);
    }
  }

  void process(const ProcessArgs &args) override {
    // --- Speed ---
    float knobSpeed = (params[SPEED_PARAM].getValue() + 1.f) * 2.5f;
    float speedCV = inputs[SPEEDCVIN_INPUT].isConnected()
                        ? inputs[SPEEDCVIN_INPUT].getVoltage()
                        : 0.f;
    speedCV = std::clamp(speedCV * 0.5f, -5.f, 5.f);
    float combinedSpeed = knobSpeed + speedCV;
    float normSpeed = std::clamp(combinedSpeed / 5.f, 0.f, 1.f);
    float speed = SPEED_MIN + normSpeed * (SPEED_MAX - SPEED_MIN);

    // --- Length ---
    float knobLength = params[LENGTH_PARAM].getValue() * 5.f;
    float lengthCV = inputs[LENGTHCVIN_INPUT].isConnected()
                         ? inputs[LENGTHCVIN_INPUT].getVoltage()
                         : 0.f;
    lengthCV = std::clamp(lengthCV * 0.5f, -5.f, 5.f);
    float combinedLength = knobLength + lengthCV;
    float normLength = std::clamp(combinedLength / 5.f, 0.f, 1.f);
    float lengthRatio = LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);

    processVoice(args, congaLVoice, CONGALTRIGIN_INPUT, CONGALPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, congaHVoice, CONGAHTRIGIN_INPUT, CONGAHPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, congaHMVoice, CONGAHMTRIGIN_INPUT, CONGAHMPUSH_PARAM,
                 speed, lengthRatio);
    processVoice(args, bongoLVoice, BONGOLTRIGIN_INPUT, BONGOLPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, bongoHVoice, BONGOHTRIGIN_INPUT, BONGOHPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, timbaleLVoice, TIMBALELTRIGIN_INPUT, TIMBALELPUSH_PARAM,
                 speed, lengthRatio);
    processVoice(args, timbaleHVoice, TIMBALEHTRIGIN_INPUT, TIMBALEHPUSH_PARAM,
                 speed, lengthRatio);
    processVoice(args, agogoLVoice, AGOGOLTRIGIN_INPUT, AGOGOLPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, agogoHVoice, AGOGOHTRIGIN_INPUT, AGOGOHPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, maracaVoice, MARACATRIGIN_INPUT, MARACAPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, cabasaVoice, CABASATRIGIN_INPUT, CABASAPUSH_PARAM, speed,
                 lengthRatio);
    processVoice(args, whistleVoice, WHISTLETRIGIN_INPUT, WHISTLEPUSH_PARAM,
                 speed, lengthRatio);

    // --- Sum output ---
    float mainVol = params[MAINVOL_PARAM].getValue();
    float busSum = 0.f;
    Voice *allVoices[] = {&congaLVoice,   &congaHVoice, &congaHMVoice,
                          &bongoLVoice,   &bongoHVoice, &timbaleLVoice,
                          &timbaleHVoice, &agogoLVoice, &agogoHVoice,
                          &maracaVoice,   &cabasaVoice, &whistleVoice};
    for (Voice *v : allVoices) {
      if (!outputs[v->outputId].isConnected()) {
        busSum += outputs[v->outputId].getVoltage();
      }
    }
    outputs[SUM_OUTPUT].setVoltage(
        std::clamp(busSum * mainVol * 0.75f, -10.f, 10.f));
  }
};

struct SehvenTooWidget : ModuleWidget {
  SehvenTooWidget(SehvenToo *module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/panels/SehvenToo.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                                          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<Davies1900hBlack>(
        mm2px(Vec(16.799, 15.501)), module, SehvenToo::LENGTH_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(
        mm2px(Vec(38.799, 15.501)), module, SehvenToo::SPEED_PARAM));
    addParam(createParamCentered<Davies1900hBlack>(
        mm2px(Vec(59.351, 15.501)), module, SehvenToo::MAINVOL_PARAM));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(8.798, 42.002)), module,
                                           SehvenToo::CONGALPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(8.798, 42.002)), module, SehvenToo::CONGAL_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(20.5, 42.002)), module,
                                           SehvenToo::CONGAHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(20.5, 42.002)), module, SehvenToo::CONGAH_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(32.3, 42.002)), module,
                                           SehvenToo::CONGAHMPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(32.3, 42.002)), module, SehvenToo::CONGAHM_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(43.998, 42.002)), module,
                                           SehvenToo::BONGOLPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(43.998, 42.002)), module, SehvenToo::BONGOL_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(55.7, 42.002)), module,
                                           SehvenToo::BONGOHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(55.7, 42.002)), module, SehvenToo::BONGOH_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(67.501, 42.002)), module,
                                           SehvenToo::TIMBALELPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(67.501, 42.002)), module, SehvenToo::TIMBALEL_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(8.798, 84.999)), module,
                                           SehvenToo::TIMBALEHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(8.798, 84.999)), module, SehvenToo::TIMBALEH_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(20.5, 84.999)), module,
                                           SehvenToo::AGOGOLPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(20.5, 84.999)), module, SehvenToo::AGOGOL_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(32.3, 84.999)), module,
                                           SehvenToo::AGOGOHPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(32.3, 84.999)), module, SehvenToo::AGOGOH_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(43.998, 84.999)), module,
                                           SehvenToo::MARACAPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(43.998, 84.999)), module, SehvenToo::MARACA_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(55.7, 84.999)), module,
                                           SehvenToo::CABASAPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(55.7, 84.999)), module, SehvenToo::CABASA_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(67.501, 84.999)), module,
                                           SehvenToo::WHISTLEPUSH_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(67.501, 84.999)), module, SehvenToo::WHISTLE_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.849, 30.801)), module,
                                             SehvenToo::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.851, 30.801)), module,
                                             SehvenToo::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.798, 56.0)), module,
                                             SehvenToo::CONGALTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.5, 56.0)), module,
                                             SehvenToo::CONGAHTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.3, 56.0)), module,
                                             SehvenToo::CONGAHMTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.998, 56.0)), module,
                                             SehvenToo::BONGOLTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.7, 56.0)), module,
                                             SehvenToo::BONGOHTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67.501, 56.0)), module,
                                             SehvenToo::TIMBALELTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.798, 99.001)), module,
                                             SehvenToo::TIMBALEHTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.5, 99.001)), module,
                                             SehvenToo::AGOGOLTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.3, 99.001)), module,
                                             SehvenToo::AGOGOHTRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.998, 99.001)), module,
                                             SehvenToo::MARACATRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.7, 99.001)), module,
                                             SehvenToo::CABASATRIGIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67.501, 99.001)), module,
                                             SehvenToo::WHISTLETRIGIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(59.351, 30.801)),
                                               module, SehvenToo::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(8.798, 70.002)), module, SehvenToo::CONGALOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.5, 70.002)), module,
                                               SehvenToo::CONGAHOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.3, 70.002)), module,
                                               SehvenToo::CONGAHMOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(43.998, 70.002)), module, SehvenToo::BONGOLOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.7, 70.002)), module,
                                               SehvenToo::BONGOHOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(67.501, 70.002)), module, SehvenToo::TIMBALELOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(8.798, 112.999)), module, SehvenToo::TIMBALEHOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(20.5, 112.999)), module, SehvenToo::AGOGOLOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(32.3, 112.999)), module, SehvenToo::AGOGOHOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(43.998, 112.999)), module, SehvenToo::MARACAOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(55.7, 112.999)), module, SehvenToo::CABASAOUT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(67.501, 112.999)), module, SehvenToo::WHISTLEOUT_OUTPUT));
  }
};

Model *modelSehvenToo = createModel<SehvenToo, SehvenTooWidget>("SehvenToo");
