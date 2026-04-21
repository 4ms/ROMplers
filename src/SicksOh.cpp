#include "SicksOhSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SicksOh : DrumMachineBaseModule<SicksOh> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", SOKick, 2.5f},
		NamedSample{"Snare", SOSnare, 2.5f},
		NamedSample{"Tom Lo", SOTomL, 2.5f},
		NamedSample{"Tom Hi", SOTomH, 2.5f},
		NamedSample{"Closed Hat", SOClosedHat, 2.5f},
		NamedSample{"Open Hat", SOOpenHat, 2.5f},
		NamedSample{"Cymbal", SOCym, 2.5f},
	};
};

struct SicksOhWidget : ModuleWidget {
	SicksOhWidget(SicksOh *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SicksOh.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module, SicksOh::LENGTH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module, SicksOh::SPEED_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(44.2, 12.45)), module, SicksOh::MAINVOL_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 37.0)), module, SicksOh::DRUM0_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 37.0)), module, SicksOh::DRUM0_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 49.499)), module, SicksOh::DRUM1_PARAM));
		addChild(
			createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 49.499)), module, SicksOh::DRUM1_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 62.001)), module, SicksOh::DRUM2_PARAM));
		addChild(
			createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 62.001)), module, SicksOh::DRUM2_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 74.5)), module, SicksOh::DRUM3_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 74.5)), module, SicksOh::DRUM3_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 86.999)), module, SicksOh::DRUM4_PARAM));
		addChild(
			createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 86.999)), module, SicksOh::DRUM4_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 99.502)), module, SicksOh::DRUM5_PARAM));
		addChild(
			createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 99.502)), module, SicksOh::DRUM5_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module, SicksOh::DRUM6_PARAM));
		addChild(
			createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 112.0)), module, SicksOh::DRUM6_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module, SicksOh::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module, SicksOh::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module, SicksOh::DRUM0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 49.499)), module, SicksOh::DRUM1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 62.001)), module, SicksOh::DRUM2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 74.5)), module, SicksOh::DRUM3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 86.999)), module, SicksOh::DRUM4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 99.502)), module, SicksOh::DRUM5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module, SicksOh::DRUM6_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module, SicksOh::SUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module, SicksOh::DRUM0_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 49.499)), module, SicksOh::DRUM1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 62.001)), module, SicksOh::DRUM2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 74.5)), module, SicksOh::DRUM3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 86.999)), module, SicksOh::DRUM4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 99.502)), module, SicksOh::DRUM5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)), module, SicksOh::DRUM6_OUTPUT));
	}
};

Model *modelSicksOh = createModel<SicksOh, SicksOhWidget>("SicksOh");
