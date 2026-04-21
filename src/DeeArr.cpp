#include "DeeArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct DeeArr : DrumMachineBaseModule<DeeArr> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", DAKick, 2.5f},
		NamedSample{"Snare", DASnare, 2.5f},
		NamedSample{"Hat", DAHat, 2.5f},
		NamedSample{"Rim", DARim, 2.5f},
	};
};

struct DeeArrWidget : ModuleWidget {
	DeeArrWidget(DeeArr *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/DeeArr.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 19.001)), module, DeeArr::SPEED_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(9.751, 29.2)), module, DeeArr::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(41.25, 29.2)), module, DeeArr::MAINVOL_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67.0)), module, DeeArr::DRUM0_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 67.0)), module, DeeArr::DRUM0_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82.0)), module, DeeArr::DRUM1_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 82.0)), module, DeeArr::DRUM1_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97.0)), module, DeeArr::DRUM2_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 97.0)), module, DeeArr::DRUM2_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module, DeeArr::DRUM3_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 112.0)), module, DeeArr::DRUM3_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 36.499)), module, DeeArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.751, 47.001)), module, DeeArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module, DeeArr::DRUM0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module, DeeArr::DRUM1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module, DeeArr::DRUM2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module, DeeArr::DRUM3_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(41.25, 47.001)), module, DeeArr::SUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module, DeeArr::DRUM0_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module, DeeArr::DRUM1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module, DeeArr::DRUM2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)), module, DeeArr::DRUM3_OUTPUT));
	}
};

Model *modelDeeArr = createModel<DeeArr, DeeArrWidget>("DeeArr");
