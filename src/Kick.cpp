#include "plugin.hpp"

struct Kick: Module {
	enum ParamId {
		SAMPLE_PARAM,
		PITCH_PARAM,
		DECAY_PARAM,
		KICKPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SAMPLECVIN_INPUT,
		PITCHCVIN_INPUT,
		DECAYCVIN_INPUT,
		TRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		LIGHTS_LEN
	};

	Kick() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configSwitch(SAMPLE_PARAM, 0.f, 49.f, 0.f, "Sample", {"1", "2", "3", "4", "5", "6", "7", 
            "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", 
            "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35",
             "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "50"});
        configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "Pitch", "%", 0.f, 100.f);
        configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", "ms", 0.f, 100.f);
        configInput(SAMPLECVIN_INPUT, "Sample CV");
        configInput(PITCHCVIN_INPUT, "Pitch CV");
        configInput(DECAYCVIN_INPUT, "Decay CV");
        configInput(TRIGIN_INPUT, "Trig");
        configOutput(AUDIOOUT_OUTPUT, "Audio");
    }

	void process(const ProcessArgs& args) override {
	}
};


struct KickWidget : ModuleWidget {
	KickWidget(Kick* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Kick_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 21.792)), module, Kick::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 45.818)), module, Kick::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 69.85)), module, Kick::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 95.162)), module, Kick::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 95.162)), module, Kick::KICK_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 35.399)), module, Kick::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 59.903)), module, Kick::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 85.479)), module, Kick::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 113.419)), module, Kick::TRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.317, 113.419)), module, Kick::AUDIOOUT_OUTPUT));
	}
};


Model* modelKick = createModel<Kick, KickWidget>("Kick");