#include "plugin.hpp"
#include "KayOneSamples.hpp"

struct KayOne : Module {
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
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		TOMLOUT_OUTPUT,
		TOMHOUT_OUTPUT,
		CLOUT_OUTPUT,
		OHOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE_LIGHT,
		TOML_LIGHT,
		TOMH_LIGHT,
		CL_LIGHT,
		OH_LIGHT,
		LIGHTS_LEN
	};

	struct Voice {
		bool lastInputTrigger = false;
		bool lastButtonTrigger = false;
	
		float samplePos = 0.f;
		bool playing = false;
		bool lastTriggerState = false;
		const unsigned char* rawData = nullptr;
		int sampleLength = 0;
		int outputId = 0;
		int lightId = -1;  
	
		int16_t readSample16(int index) {
			return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
		}
	};
	

	Voice kickVoice;
	Voice snareVoice;
	Voice tomLoVoice;
	Voice tomHiVoice;
	Voice closedHatVoice;
	Voice openHatVoice;

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 1.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	KayOne() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
		configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
		configSwitch(CLPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OHPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop Gate");

		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(TOMLTRIG_INPUT, "Tom Lo Trig");
		configInput(TOMHTRIG_INPUT, "Tom Hi Trig");
		configInput(CLTRIG_INPUT, "Closed Hat Trig");
		configInput(OHTRIG_INPUT, "Open Hat Trig");

		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(TOMLOUT_OUTPUT, "Tom Lo");
		configOutput(TOMHOUT_OUTPUT, "Tom Hi");
		configOutput(CLOUT_OUTPUT, "Closed Hat");
		configOutput(OHOUT_OUTPUT, "Open Hat");

		kickVoice.rawData = Kick;
		kickVoice.sampleLength = sizeof(Kick) / 2;
		kickVoice.outputId = KICKOUT_OUTPUT;
		kickVoice.lightId = KICK_LIGHT;

		snareVoice.rawData = Snare;
		snareVoice.sampleLength = sizeof(Snare) / 2;
		snareVoice.outputId = SNAREOUT_OUTPUT;
		snareVoice.lightId = SNARE_LIGHT;

		tomLoVoice.rawData = TomLo;
		tomLoVoice.sampleLength = sizeof(TomLo) / 2;
		tomLoVoice.outputId = TOMLOUT_OUTPUT;
		tomLoVoice.lightId = TOML_LIGHT;

		tomHiVoice.rawData = TomHi;
		tomHiVoice.sampleLength = sizeof(TomHi) / 2;
		tomHiVoice.outputId = TOMHOUT_OUTPUT;
		tomHiVoice.lightId = TOMH_LIGHT;

		closedHatVoice.rawData = ClosedHat;
		closedHatVoice.sampleLength = sizeof(ClosedHat) / 2;
		closedHatVoice.outputId = CLOUT_OUTPUT;
		closedHatVoice.lightId = CL_LIGHT;

		openHatVoice.rawData = OpenHat;
		openHatVoice.sampleLength = sizeof(OpenHat) / 2;
		openHatVoice.outputId = OHOUT_OUTPUT;
		openHatVoice.lightId = OH_LIGHT;
	}

	void process(const ProcessArgs& args) override {
		// Speed calculation
		float knobSpeed = 0.01f + params[SPEED_PARAM].getValue() * (1.0f - 0.01f);
		float speedCV = clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float speedOffset = (speedCV / 5.f) * 0.5f;
		float normSpeed = clamp(knobSpeed + speedOffset, 0.01f, 1.0f);
		float speed = SPEED_LOW + (normSpeed - 0.01f) * ((SPEED_HIGH - SPEED_LOW) / (1.0f - 0.01f));

		// Length calculation
		float knobLength = params[LENGTH_PARAM].getValue();
		float lengthCV = clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float lengthOffset = (lengthCV / 5.f) * 0.5f;
		float normLength = clamp(knobLength + lengthOffset, 0.1f, 1.0f);
		float lengthRatio = LENGTH_MIN + (normLength - 0.1f) * ((LENGTH_MAX - LENGTH_MIN) / (1.0f - 0.1f));

		bool baseLoop = params[LOOP_PARAM].getValue() > 0.5f;
		float loopCV = inputs[LOOPCVIN_INPUT].getVoltage();
		bool loopEnabled = baseLoop;
		
		// Apply CV override logic
		if (!baseLoop && loopCV > 1.f) {
			loopEnabled = true;
		}
		else if (baseLoop && loopCV < -1.f) {
			loopEnabled = false;
		}
		
		// Process each voice
		processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, KICKOUT_OUTPUT, speed, lengthRatio, loopEnabled);
		processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, SNAREOUT_OUTPUT, speed, lengthRatio, loopEnabled);
		processVoice(args, tomLoVoice, TOMLTRIG_INPUT, TOMLPUSH_PARAM, TOMLOUT_OUTPUT, speed, lengthRatio, loopEnabled);
		processVoice(args, tomHiVoice, TOMHTRIG_INPUT, TOMHPUSH_PARAM, TOMHOUT_OUTPUT, speed, lengthRatio, loopEnabled);
		processVoice(args, closedHatVoice, CLTRIG_INPUT, CLPUSH_PARAM, CLOUT_OUTPUT, speed, lengthRatio, loopEnabled);
		processVoice(args, openHatVoice, OHTRIG_INPUT, OHPUSH_PARAM, OHOUT_OUTPUT, speed, lengthRatio, loopEnabled);
	}

	void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId, int outputId, float speed, float lengthRatio, bool loopEnabled) {
		// Detect triggers
		bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
		bool buttonTrigger = params[pushParamId].getValue() > 0.5f;
	
		bool inputRising = inputTrigger && !voice.lastInputTrigger;
		bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;
	
		if (inputRising || buttonRising) {
			voice.playing = true;
			voice.samplePos = 0.f;
	
			// Light full brightness on trigger
			if (voice.lightId >= 0)
				lights[voice.lightId].setBrightness(1.0f);
		}
	
		voice.lastInputTrigger = inputTrigger;
		voice.lastButtonTrigger = buttonTrigger;
	
		// Loop mode
		if (loopEnabled && !voice.playing) {
			voice.playing = true;
			voice.samplePos = 0.f;
		}
	
		int maxSamplesToPlay = (int)(voice.sampleLength * lengthRatio);
	
		if (voice.playing) {
			int idx = (int)voice.samplePos;
			if (idx < maxSamplesToPlay) {
				int16_t sampleInt = voice.readSample16(idx);
				float sample = (float)sampleInt / 32768.f;
				outputs[outputId].setVoltage(sample * 5.f);
				voice.samplePos += speed;
			} else {
				if (loopEnabled) {
					voice.samplePos = 0.f;
				} else {
					voice.playing = false;
					outputs[outputId].setVoltage(0.f);
				}
			}
		} else {
			outputs[outputId].setVoltage(0.f);
		}
	
		// Smooth light fade-out
		if (voice.lightId >= 0)
			lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
	}	
};

struct KayOneWidget : ModuleWidget {
	KayOneWidget(KayOne* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/KayOne_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(20.32, 15.958)), module, KayOne::SPEED_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(8.867, 35.67)), module, KayOne::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(30.511, 35.67)), module, KayOne::LOOP_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 63.771)), module, KayOne::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 63.771)), module, KayOne::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 73.417)), module, KayOne::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 73.417)), module, KayOne::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 83.302)), module, KayOne::TOMLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 83.302)), module, KayOne::TOML_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 93.19)), module, KayOne::TOMHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 93.19)), module, KayOne::TOMH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 103.551)), module, KayOne::CLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 103.551)), module, KayOne::CL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(5.692, 114.064)), module, KayOne::OHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(5.692, 114.064)), module, KayOne::OH_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 29.153)), module, KayOne::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.867, 49.49)), module, KayOne::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.511, 49.49)), module, KayOne::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 63.771)), module, KayOne::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 73.417)), module, KayOne::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 83.302)), module, KayOne::TOMLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 93.19)), module, KayOne::TOMHTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 103.551)), module, KayOne::CLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.495, 114.064)), module, KayOne::OHTRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 63.771)), module, KayOne::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 73.417)), module, KayOne::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 83.302)), module, KayOne::TOMLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 93.19)), module, KayOne::TOMHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 103.551)), module, KayOne::CLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.627, 114.064)), module, KayOne::OHOUT_OUTPUT));
	}
};


Model* modelKayOne = createModel<KayOne, KayOneWidget>("KayOne");