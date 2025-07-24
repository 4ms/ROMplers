#include "plugin.hpp"
#include "DeeArrSamples.hpp" // Ensure this includes DAKick, DASnare, DAHat, DARim

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

	struct Voice {
		bool lastInputTrigger = false;
		bool lastButtonTrigger = false;
		float samplePos = 0.f;
		bool playing = false;
		const unsigned char* rawData = nullptr;
		int sampleLength = 0;
		int outputId = 0;
		int lightId = -1;

		int16_t readSample16(int index) {
			return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
		}
	};

	Voice kickVoice, snareVoice, hatVoice, rimVoice;

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 2.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	DeeArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(HATPUSH_PARAM, 0.f, 1.f, 0.f, "Hat Trig", {"Off", "On"});
		configSwitch(RIMPUSH_PARAM, 0.f, 1.f, 0.f, "Rim Trig", {"Off", "On"});

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(HATTRIGIN_INPUT, "Hat Trig");
		configInput(RIMTRIGIN_INPUT, "Rim Trig");

		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(HATOUT_OUTPUT, "Hat");
		configOutput(RIMOUT_OUTPUT, "Rim");

		kickVoice  = createVoice(DAKick, sizeof(DAKick), KICKOUT_OUTPUT, KICK_LIGHT);
		snareVoice = createVoice(DASnare, sizeof(DASnare), SNAREOUT_OUTPUT, SNARE_LIGHT);
		hatVoice   = createVoice(DAHat, sizeof(DAHat), HATOUT_OUTPUT, HAT_LIGHT);
		rimVoice   = createVoice(DARim, sizeof(DARim), RIMOUT_OUTPUT, RIM_LIGHT);
	}

	Voice createVoice(const unsigned char* data, size_t size, int outputId, int lightId) {
		Voice v;
		v.rawData = data;
		v.sampleLength = size / 2;
		v.outputId = outputId;
		v.lightId = lightId;
		return v;
	}

	void process(const ProcessArgs& args) override {
		float knobSpeed = 0.01f + params[SPEED_PARAM].getValue() * (1.0f - 0.01f);
		float speedCV = clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float speedOffset = (speedCV / 5.f) * 0.5f;
		float normSpeed = clamp(knobSpeed + speedOffset, 0.01f, 1.0f);
		float speed = SPEED_LOW + (normSpeed - 0.01f) * ((SPEED_HIGH - SPEED_LOW) / (1.0f - 0.01f));

		float knobLength = params[LENGTH_PARAM].getValue();
		float lengthCV = clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float lengthOffset = (lengthCV / 5.f) * 0.5f;
		float normLength = clamp(knobLength + lengthOffset, 0.1f, 1.0f);
		float lengthRatio = LENGTH_MIN + (normLength - 0.1f) * ((LENGTH_MAX - LENGTH_MIN) / (1.0f - 0.1f));

		bool baseLoop = params[LOOP_PARAM].getValue() > 0.5f;
		bool loopEnabled = baseLoop || (!baseLoop && inputs[LOOPCVIN_INPUT].getVoltage() > 1.f);

		processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, hatVoice, HATTRIGIN_INPUT, HATPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, rimVoice, RIMTRIGIN_INPUT, RIMPUSH_PARAM, speed, lengthRatio, loopEnabled);
	}

	void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId, float speed, float lengthRatio, bool loopEnabled) {
		bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
		bool buttonTrigger = params[pushParamId].getValue() > 0.5f;

		bool inputRising = inputTrigger && !voice.lastInputTrigger;
		bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;

		if (inputRising || buttonRising || (loopEnabled && !voice.playing)) {
			voice.playing = true;
			voice.samplePos = 0.f;
		}

		voice.lastInputTrigger = inputTrigger;
		voice.lastButtonTrigger = buttonTrigger;

		int maxSamplesToPlay = (int)(voice.sampleLength * lengthRatio);

		if (voice.playing) {
			int idx = (int)voice.samplePos;
			if (idx < maxSamplesToPlay) {
				int16_t sampleInt = voice.readSample16(idx);
				float sample = (float)sampleInt / 32768.f;
				outputs[voice.outputId].setVoltage(sample * 5.f);
				voice.samplePos += speed;
			} else {
				if (loopEnabled) {
					voice.samplePos = 0.f;
				} else {
					voice.playing = false;
					outputs[voice.outputId].setVoltage(0.f);
				}
			}
		} else {
			outputs[voice.outputId].setVoltage(0.f);
		}

		if (voice.lightId >= 0) {
			if ((inputRising || buttonRising || (loopEnabled && !voice.playing))) {
				lights[voice.lightId].setBrightness(1.f);
			}
			lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
		}
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