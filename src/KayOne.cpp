#include "KayOneSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct KayOne : DrumMachineBaseModule<KayOne> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", SKKick, 2.f},
		NamedSample{"Snare", SKSnare, 2.f},
		NamedSample{"Tom Lo", SKTomLo, 2.f},
		NamedSample{"Tom Hi", SKTomHi, 2.f},
		NamedSample{"Closed Hat", SKClosedHat, 2.f},
		NamedSample{"Open Hat", SKOpenHat, 2.f},
	};
};

struct KayOneWidget : ModuleWidget {
	KayOneWidget(KayOne *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/KayOne.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module, KayOne::LENGTH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module, KayOne::SPEED_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(44.2, 12.45)), module, KayOne::MAINVOL_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 37.0)), module, KayOne::DRUM0_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 37.0)), module, KayOne::DRUM0_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 52.0)), module, KayOne::DRUM1_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 52.0)), module, KayOne::DRUM1_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67.0)), module, KayOne::DRUM2_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 67.0)), module, KayOne::DRUM2_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82.0)), module, KayOne::DRUM3_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 82.0)), module, KayOne::DRUM3_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97.0)), module, KayOne::DRUM4_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 97.0)), module, KayOne::DRUM4_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module, KayOne::DRUM5_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 112.0)), module, KayOne::DRUM5_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module, KayOne::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module, KayOne::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module, KayOne::DRUM0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 52.0)), module, KayOne::DRUM1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module, KayOne::DRUM2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module, KayOne::DRUM3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module, KayOne::DRUM4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module, KayOne::DRUM5_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module, KayOne::SUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module, KayOne::DRUM0_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 52.0)), module, KayOne::DRUM1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module, KayOne::DRUM2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module, KayOne::DRUM3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module, KayOne::DRUM4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)), module, KayOne::DRUM5_OUTPUT));
	}
};

Model *modelKayOne = createModel<KayOne, KayOneWidget>("KayOne");
