#include "plugin.hpp"


struct DeeArr : Module {
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNAREPUSH_PARAM,
		HATPUSH_PARAM,
		RIMPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		LOOPCVIN_INPUT,
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
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE_LIGHT,
		HAT_LIGHT,
		RIM_LIGHT,
		LIGHTS_LEN
	};

	DeeArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(HATPUSH_PARAM, 0.f, 1.f, 0.f, "Hat Trig", {"Off", "On"});
		configParam(RIMPUSH_PARAM, 0.f, 1.f, 0.f, "Rim Trig", {"Off", "On"});
		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick CV");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(HATTRIGIN_INPUT, "Hat Trig");
		configInput(RIMTRIGIN_INPUT, "Rim Trig");
		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(HATOUT_OUTPUT, "Hat");
		configOutput(RIMOUT_OUTPUT, "Rim");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct DeeArrWidget : ModuleWidget {
	DeeArrWidget(DeeArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/DeeArr_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(20.32, 15.958)), module, DeeArr::SPEED_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(8.867, 35.67)), module, DeeArr::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(30.511, 35.67)), module, DeeArr::LOOP_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 63.771)), module, DeeArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 63.771)), module, DeeArr::KICK_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 73.417)), module, DeeArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 73.471)), module, DeeArr::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 83.302)), module, DeeArr::HATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 83.302)), module, DeeArr::HAT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 93.19)), module, DeeArr::RIMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 93.19)), module, DeeArr::RIM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 29.153)), module, DeeArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.867, 49.49)), module, DeeArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.511, 49.49)), module, DeeArr::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 63.771)), module, DeeArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 73.417)), module, DeeArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 83.302)), module, DeeArr::HATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 93.19)), module, DeeArr::RIMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 63.771)), module, DeeArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 73.417)), module, DeeArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 83.302)), module, DeeArr::HATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 93.19)), module, DeeArr::RIMOUT_OUTPUT));
	}
};


Model* modelDeeArr = createModel<DeeArr, DeeArrWidget>("DeeArr");