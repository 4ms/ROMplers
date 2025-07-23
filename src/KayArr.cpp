#include "plugin.hpp"


struct KayArr : Module {
	enum ParamId {
		SPEED__PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNAREPUSH_PARAM,
		CLOSEDHATPUSH_PARAM,
		OPENHATPUSH_PARAM,
		TOMPUSH_PARAM,
		CONGAPUSH_PARAM,
		CLAVEPUSH_PARAM,
		RIMSHOTPUSH_PARAM,
		COWBELLPUSH_PARAM,
		CYMPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		LOOPGATEIN_INPUT,
		KICKTRIGIN_INPUT,
		SNARETRIGIN_INPUT,
		CLOSEDHATTRIGIN_INPUT,
		OPENHATTRIGIN_INPUT,
		TOMTRIGIN_INPUT,
		CONGATRIGIN_INPUT,
		CLAVETRIGIN_INPUT,
		RIMSHOTTRIGIN_INPUT,
		COWBELLTRIGIN_INPUT,
		CYMTRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		CLOSEDHATOUT_OUTPUT,
		OPENHATOUT_OUTPUT,
		TOMOUT_OUTPUT,
		CONGAOUT_OUTPUT,
		CLAVEOUT_OUTPUT,
		RIMSHOTOUT_OUTPUT,
		COWBELLOUT_OUTPUT,
		CYMOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	KayArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED__PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(CLOSEDHATPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OPENHATPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});
		configSwitch(TOMPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Trig", {"Off", "On"});
		configSwitch(CONGAPUSH_PARAM, 0.f, 1.f, 0.f, "Conga Trig", {"Off", "On"});
		configSwitch(CLAVEPUSH_PARAM, 0.f, 1.f, 0.f, "Clave Trig", {"Off", "On"});
		configSwitch(RIMSHOTPUSH_PARAM, 0.f, 1.f, 0.f, "Rimshot Trig", {"Off", "On"});
		configSwitch(COWBELLPUSH_PARAM, 0.f, 1.f, 0.f, "Cowbell Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymball Trig", {"Off", "On"});
		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPGATEIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(CLOSEDHATTRIGIN_INPUT, "Closed Hat Trig");
		configInput(OPENHATTRIGIN_INPUT, "Open Hat Trig");
		configInput(TOMTRIGIN_INPUT, "Tom Trig");
		configInput(CONGATRIGIN_INPUT, "Conga Trig");
		configInput(CLAVETRIGIN_INPUT, "Clave Trig");
		configInput(RIMSHOTTRIGIN_INPUT, "Rimshot Trig");
		configInput(COWBELLTRIGIN_INPUT, "Cowbell Trig");
		configInput(CYMTRIGIN_INPUT, "Cymball Trig");
		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(CLOSEDHATOUT_OUTPUT, "Closed Hat");
		configOutput(OPENHATOUT_OUTPUT, "Open Hat");
		configOutput(TOMOUT_OUTPUT, "Tom");
		configOutput(CONGAOUT_OUTPUT, "Conga");
		configOutput(CLAVEOUT_OUTPUT, "Clave");
		configOutput(RIMSHOTOUT_OUTPUT, "Rimshot");
		configOutput(COWBELLOUT_OUTPUT, "Cowbell");
		configOutput(CYMOUT_OUTPUT, "Cymbal");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct KayArrWidget : ModuleWidget {
	KayArrWidget(KayArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/KayArr_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 21.308)), module, KayArr::SPEED__PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 33.508)), module, KayArr::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(41.683, 33.508)), module, KayArr::LOOP_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.0, 59.14)), module, KayArr::KICKPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(16.49, 59.14)), module, KayArr::SNAREPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(25.4, 59.14)), module, KayArr::CLOSEDHATPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(35.318, 59.14)), module, KayArr::OPENHATPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(44.699, 59.14)), module, KayArr::TOMPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.0, 94.743)), module, KayArr::CONGAPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(16.49, 94.743)), module, KayArr::CLAVEPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(25.4, 94.743)), module, KayArr::RIMSHOTPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(35.318, 94.743)), module, KayArr::COWBELLPUSH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(44.699, 94.743)), module, KayArr::CYMPUSH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 33.97)), module, KayArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 45.349)), module, KayArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.683, 45.349)), module, KayArr::LOOPGATEIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 71.357)), module, KayArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 71.357)), module, KayArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 71.357)), module, KayArr::CLOSEDHATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 71.357)), module, KayArr::OPENHATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 71.357)), module, KayArr::TOMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 106.959)), module, KayArr::CONGATRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 106.959)), module, KayArr::CLAVETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 106.959)), module, KayArr::RIMSHOTTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 106.959)), module, KayArr::COWBELLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 106.959)), module, KayArr::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 83.813)), module, KayArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 83.813)), module, KayArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 83.813)), module, KayArr::CLOSEDHATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 83.813)), module, KayArr::OPENHATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 83.813)), module, KayArr::TOMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 118.357)), module, KayArr::CONGAOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 118.357)), module, KayArr::CLAVEOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 118.357)), module, KayArr::RIMSHOTOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 118.357)), module, KayArr::COWBELLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 118.357)), module, KayArr::CYMOUT_OUTPUT));
	}
};


Model* modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");