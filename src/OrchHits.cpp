#include "plugin.hpp"


struct OrchHits : Module {
	enum ParamId {
		SAMPLE_PARAM,
		PITCH_PARAM,
		DECAY_PARAM,
		PUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SAMPLECVIN_INPUT,
		PITCHCVIN_INPUT,
		DECAYCVIN_INPUT,
		TRIGIN_INPUT,
		VOLCVIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	OrchHits() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SAMPLE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "");
		configInput(SAMPLECVIN_INPUT, "");
		configInput(PITCHCVIN_INPUT, "");
		configInput(DECAYCVIN_INPUT, "");
		configInput(TRIGIN_INPUT, "");
		configInput(VOLCVIN_INPUT, "");
		configOutput(AUDIOOUT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct OrchHitsWidget : ModuleWidget {
	OrchHitsWidget(OrchHits* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/OrchHits.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 16.052)), module, OrchHits::SAMPLE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 39.468)), module, OrchHits::PITCH_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 63.5)), module, OrchHits::DECAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 88.812)), module, OrchHits::PUSH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 30.138)), module, OrchHits::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 53.553)), module, OrchHits::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.129)), module, OrchHits::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 104.952)), module, OrchHits::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 104.952)), module, OrchHits::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 118.71)), module, OrchHits::AUDIOOUT_OUTPUT));
	}
};


Model* modelOrchHits = createModel<OrchHits, OrchHitsWidget>("OrchHits");