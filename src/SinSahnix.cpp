#include "SinSahnixSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SinSahnix : DrumMachineBaseModule<SinSahnix> {
  static constexpr auto drums = std::array{
      NamedSample{"Kick", SSKick, 2.f},
      NamedSample{"Snare", SSSnare, 2.f},
      NamedSample{"Tom Lo", SSTomL, 2.f},
      NamedSample{"Tom Mid", SSTomM, 2.f},
      NamedSample{"Tom Hi", SSTomH, 2.f},
      NamedSample{"Cymbal", SSCymbal, 2.f},
  };
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
                                           SinSahnix::DRUM0_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 37)), module, SinSahnix::DRUM0_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 52)), module,
                                           SinSahnix::DRUM1_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 52)), module, SinSahnix::DRUM1_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67)), module,
                                           SinSahnix::DRUM2_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 67)), module, SinSahnix::DRUM2_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82)), module,
                                           SinSahnix::DRUM3_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 82)), module, SinSahnix::DRUM3_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97)), module,
                                           SinSahnix::DRUM4_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 97)), module, SinSahnix::DRUM4_LIGHT));

    addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112)), module,
                                           SinSahnix::DRUM5_PARAM));
    addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
        mm2px(Vec(7.751, 112)), module, SinSahnix::DRUM5_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module,
                                             SinSahnix::LENGTHCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module,
                                             SinSahnix::SPEEDCVIN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module,
                                             SinSahnix::DRUM0_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 52.0)), module,
                                             SinSahnix::DRUM1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module,
                                             SinSahnix::DRUM2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module,
                                             SinSahnix::DRUM3_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module,
                                             SinSahnix::DRUM4_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module,
                                             SinSahnix::DRUM5_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module,
                                               SinSahnix::SUM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module,
                                               SinSahnix::DRUM0_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 52.0)), module,
                                               SinSahnix::DRUM1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module,
                                               SinSahnix::DRUM2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module,
                                               SinSahnix::DRUM3_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module,
                                               SinSahnix::DRUM4_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(43.998, 112.0)), module, SinSahnix::DRUM5_OUTPUT));
  }
};

Model *modelSinSahnix = createModel<SinSahnix, SinSahnixWidget>("SinSahnix");
