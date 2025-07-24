#include "plugin.hpp"
#include "SicksOhSamples.hpp"

struct SicksOh : Module {
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNAREPUSH_PARAM,
		TOMLPUSH_PARAM,
		TOMHPUSH_PARAM,
		CLPUSH_PARAM,
		OHPUSH_PARAM,
		CYMPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT,
		SNARETRIGIN_INPUT,
		TOMLTRIG_INPUT,
		TOMHTRIG_INPUT,
		CLTRIG_INPUT,
		OHTRIG_INPUT,
		CYMTRIG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		TOMLOUT_OUTPUT,
		TOMHOUT_OUTPUT,
		CLOUT_OUTPUT,
		OHOUT_OUTPUT,
		CYMOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE_LIGHT,
		TOML_LIGHT,
		TOMH_LIGHT,
		CL_LIGHT,
		OH_LIGHT,
		CYM_LIGHT,
		LIGHTS_LEN
	};

	SicksOh() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED_PARAM, 0.f, 1.f, 0.f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 0.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
		configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
		configSwitch(CLPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OHPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});
		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(TOMLTRIG_INPUT, "Tom Lo Trig");
		configInput(TOMHTRIG_INPUT, "Tom Hi Trig");
		configInput(CLTRIG_INPUT, "Closed Hat Trig");
		configInput(OHTRIG_INPUT, "Open Hat Trig");
		configInput(CYMTRIG_INPUT, "Cymbal Trig");
		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(TOMLOUT_OUTPUT, "Tom Lo");
		configOutput(TOMHOUT_OUTPUT, "Tom Hi");
		configOutput(CLOUT_OUTPUT, "Closed Hat");
		configOutput(OHOUT_OUTPUT, "Open Hat");
		configOutput(CYMOUT_OUTPUT, "Cymbal");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct SicksOhWidget : ModuleWidget {
	SicksOhWidget(SicksOh* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SicksOh_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(20.32, 15.958)), module, SicksOh::SPEED_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(8.867, 24.557)), module, SicksOh::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(30.511, 24.557)), module, SicksOh::LOOP_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 53.187)), module, SicksOh::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 53.187)), module, SicksOh::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 62.834)), module, SicksOh::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 62.834)), module, SicksOh::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 72.719)), module, SicksOh::TOMLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 72.719)), module, SicksOh::TOML_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 82.607)), module, SicksOh::TOMHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 82.607)), module, SicksOh::TOMH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 92.968)), module, SicksOh::CLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 92.968)), module, SicksOh::CL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 103.481)), module, SicksOh::OHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 103.481)), module, SicksOh::OH_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 113.154)), module, SicksOh::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 113.154)), module, SicksOh::CYM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 29.153)), module, SicksOh::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.867, 38.377)), module, SicksOh::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.511, 38.377)), module, SicksOh::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 53.187)), module, SicksOh::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 62.834)), module, SicksOh::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 72.719)), module, SicksOh::TOMLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 82.607)), module, SicksOh::TOMHTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 92.968)), module, SicksOh::CLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 103.481)), module, SicksOh::OHTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 113.154)), module, SicksOh::CYMTRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 53.187)), module, SicksOh::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 62.834)), module, SicksOh::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 72.719)), module, SicksOh::TOMLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 82.607)), module, SicksOh::TOMHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 92.968)), module, SicksOh::CLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 103.481)), module, SicksOh::OHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 113.154)), module, SicksOh::CYMOUT_OUTPUT));
	}
};


Model* modelSicksOh = createModel<SicksOh, SicksOhWidget>("SicksOh");