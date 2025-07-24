#include "plugin.hpp"
#include "SinSahnixSamples.hpp"

struct SinSahnix : Module {
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
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
		LOOPCVIN_INPUT,
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

	SinSahnix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED_PARAM, 0.f, 1.f, 0.f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 0.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
		configSwitch(TOMMPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Med Trig", {"Off", "On"});
		configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});
		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(TOMLTRIGIN_INPUT, "Tom Lo Trig");
		configInput(TOMMTRIGIN_INPUT, "Tom Med Trig");
		configInput(TOMHTRIGIN_INPUT, "Tom Hi Trig");
		configInput(CYMTRIGIN_INPUT, "Cymbal Trig");
		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(TOMLOUT_OUTPUT, "Tom Lo");
		configOutput(TOMMOUT_OUTPUT, "Tom Med");
		configOutput(TOMHOUT_OUTPUT, "Tom Hi");
		configOutput(CYMOUT_OUTPUT, "Cymbal");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct SinSahnixWidget : ModuleWidget {
	SinSahnixWidget(SinSahnix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SinSahnix_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(20.32, 15.958)), module, SinSahnix::SPEED_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(8.867, 35.67)), module, SinSahnix::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(30.511, 35.67)), module, SinSahnix::LOOP_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 63.771)), module, SinSahnix::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 63.771)), module, SinSahnix::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 73.417)), module, SinSahnix::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 73.417)), module, SinSahnix::SNARE_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 83.302)), module, SinSahnix::TOMLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 83.302)), module, SinSahnix::TOML_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 93.19)), module, SinSahnix::TOMMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 93.19)), module, SinSahnix::TOMM_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 103.551)), module, SinSahnix::TOMHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 103.551)), module, SinSahnix::TOMH_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 114.064)), module, SinSahnix::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 114.064)), module, SinSahnix::CYM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 29.153)), module, SinSahnix::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.867, 49.49)), module, SinSahnix::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.511, 49.49)), module, SinSahnix::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 63.771)), module, SinSahnix::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 73.417)), module, SinSahnix::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 83.302)), module, SinSahnix::TOMLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 93.19)), module, SinSahnix::TOMMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 103.551)), module, SinSahnix::TOMHTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 114.064)), module, SinSahnix::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 63.771)), module, SinSahnix::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 73.417)), module, SinSahnix::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 83.302)), module, SinSahnix::TOMLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 93.19)), module, SinSahnix::TOMMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 103.551)), module, SinSahnix::TOMHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 114.064)), module, SinSahnix::CYMOUT_OUTPUT));
	}
};


Model* modelSinSahnix = createModel<SinSahnix, SinSahnixWidget>("SinSahnix");