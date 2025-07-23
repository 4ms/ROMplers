#include "plugin.hpp"
#include "AyysKingSamples.hpp"

struct AyysKing : Module {
	enum ParamId {
		SPEED__PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNARE1PUSH_PARAM,
		SNARE2PUSH_PARAM,
		CLOSEDHATPUSH_PARAM,
		OPENHATPUSH_PARAM,
		BONGO1PUSH_PARAM,
		BONGO2PUSH_PARAM,
		BONGO3PUSH_PARAM,
		CLAVEPUSH_PARAM,
		CYMPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT,
		SNARE1TRIGIN_INPUT,
		SNARE2TRIGIN_INPUT,
		CLOSEDHATTRIGIN_INPUT,
		OPENHATTRIGIN_INPUT,
		BONGO1TRIGIN_INPUT,
		BONGO2TRIGIN_INPUT,
		BONGO3TRIGIN_INPUT,
		CLAVETRIGIN_INPUT,
		CYMTRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNARE1OUT_OUTPUT,
		SNARE2OUT_OUTPUT,
		CLOSEDHATOUT_OUTPUT,
		OPENHATOUT_OUTPUT,
		BONGO1OUT_OUTPUT,
		BONGO2OUT_OUTPUT,
		BONGO3OUT_OUTPUT,
		CLAVEOUT_OUTPUT,
		CYMOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE1_LIGHT,
		SNARE2_LIGHT,
		CLOSEDHAT_LIGHT,
		OPENHAT_LIGHT,
		BONGO1_LIGHT,
		BONGO2_LIGHT,
		BONGO3_LIGHT,
		CLAVE_LIGHT,
		CYMBAL_LIGHT,
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

	Voice kickVoice, snare1Voice, snare2Voice;
	Voice closedHatVoice, openHatVoice;
	Voice bongo1Voice, bongo2Voice, bongo3Voice;
	Voice claveVoice, cymbalVoice;

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 3.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	AyysKing() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED__PARAM, 0.f, 1.f, 0.33f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNARE1PUSH_PARAM, 0.f, 1.f, 0.f, "Snare 1 Trig", {"Off", "On"});
		configSwitch(SNARE2PUSH_PARAM, 0.f, 1.f, 0.f, "Snare 2 Trig", {"Off", "On"});
		configSwitch(CLOSEDHATPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OPENHATPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});
		configSwitch(BONGO1PUSH_PARAM, 0.f, 1.f, 0.f, "Bongo 1 Trig", {"Off", "On"});
		configSwitch(BONGO2PUSH_PARAM, 0.f, 1.f, 0.f, "Bongo 2 Trig", {"Off", "On"});
		configSwitch(BONGO3PUSH_PARAM, 0.f, 1.f, 0.f, "Bongo 3 Trig", {"Off", "On"});
		configSwitch(CLAVEPUSH_PARAM, 0.f, 1.f, 0.f, "Clave Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARE1TRIGIN_INPUT, "Snare 1 Trig");
		configInput(SNARE2TRIGIN_INPUT, "Snare 2 Trig");
		configInput(CLOSEDHATTRIGIN_INPUT, "Closed Hat Trig");
		configInput(OPENHATTRIGIN_INPUT, "Open Hat Trig");
		configInput(BONGO1TRIGIN_INPUT, "Bongo 1 Trig");
		configInput(BONGO2TRIGIN_INPUT, "Bongo 2 Trig");
		configInput(BONGO3TRIGIN_INPUT, "Bongo 3 Trig");
		configInput(CLAVETRIGIN_INPUT, "Clave Trig");
		configInput(CYMTRIGIN_INPUT, "Cymbal Trig");

		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNARE1OUT_OUTPUT, "Snare 1");
		configOutput(SNARE2OUT_OUTPUT, "Snare 2");
		configOutput(CLOSEDHATOUT_OUTPUT, "Closed Hat");
		configOutput(OPENHATOUT_OUTPUT, "Open Hat");
		configOutput(BONGO1OUT_OUTPUT, "Bongo 1");
		configOutput(BONGO2OUT_OUTPUT, "Bongo 2");
		configOutput(BONGO3OUT_OUTPUT, "Bongo 3");
		configOutput(CLAVEOUT_OUTPUT, "Clave");
		configOutput(CYMOUT_OUTPUT, "Cymbal");

		// Initialize sample players
		kickVoice      = createVoice(AKKick,         sizeof(AKKick),         KICKOUT_OUTPUT,      KICK_LIGHT);
		snare1Voice    = createVoice(AKSnare,        sizeof(AKSnare),        SNARE1OUT_OUTPUT,    SNARE1_LIGHT);
		snare2Voice    = createVoice(AKSnare2,       sizeof(AKSnare2),       SNARE2OUT_OUTPUT,    SNARE2_LIGHT);
		closedHatVoice = createVoice(AKClosedHiHat,  sizeof(AKClosedHiHat),  CLOSEDHATOUT_OUTPUT, CLOSEDHAT_LIGHT);
		openHatVoice   = createVoice(AKOpenHiHat,    sizeof(AKOpenHiHat),    OPENHATOUT_OUTPUT,   OPENHAT_LIGHT);
		bongo1Voice    = createVoice(AKBongo,        sizeof(AKBongo),        BONGO1OUT_OUTPUT,    BONGO1_LIGHT);
		bongo2Voice    = createVoice(AKBongo2,       sizeof(AKBongo2),       BONGO2OUT_OUTPUT,    BONGO2_LIGHT);
		bongo3Voice    = createVoice(AKBongo3,       sizeof(AKBongo3),       BONGO3OUT_OUTPUT,    BONGO3_LIGHT);
		claveVoice     = createVoice(AKWood,         sizeof(AKWood),         CLAVEOUT_OUTPUT,     CLAVE_LIGHT);
		cymbalVoice    = createVoice(AKCrash,        sizeof(AKCrash),        CYMOUT_OUTPUT,       CYMBAL_LIGHT);
	}

	Voice createVoice(const unsigned char* data, size_t size, int outputId, int lightId) {
		Voice v;
		v.rawData = data;
		v.sampleLength = size / 2;
		v.outputId = outputId;
		v.lightId = lightId;
		return v;
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
			if (inputRising || buttonRising || (loopEnabled && !voice.playing)) {
				lights[voice.lightId].setBrightness(1.f);
			}
			lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
		}
	}

	void process(const ProcessArgs& args) override {
		float knobSpeed = 0.01f + params[SPEED__PARAM].getValue() * (1.0f - 0.01f);
		float speedCV = clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float normSpeed = clamp(knobSpeed + (speedCV / 5.f) * 0.5f, 0.01f, 1.0f);
		float speed = SPEED_LOW + (normSpeed - 0.01f) * ((SPEED_HIGH - SPEED_LOW) / (1.0f - 0.01f));

		float knobLength = params[LENGTH_PARAM].getValue();
		float lengthCV = clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float normLength = clamp(knobLength + (lengthCV / 5.f) * 0.5f, 0.1f, 1.0f);
		float lengthRatio = LENGTH_MIN + (normLength - 0.1f) * ((LENGTH_MAX - LENGTH_MIN) / (1.0f - 0.1f));

		bool baseLoop = params[LOOP_PARAM].getValue() > 0.5f;
		bool loopEnabled = baseLoop || (!baseLoop && inputs[LOOPCVIN_INPUT].getVoltage() > 1.f);

		processVoice(args, kickVoice,      KICKTRIGIN_INPUT,      KICKPUSH_PARAM,      speed, lengthRatio, loopEnabled);
		processVoice(args, snare1Voice,    SNARE1TRIGIN_INPUT,    SNARE1PUSH_PARAM,    speed, lengthRatio, loopEnabled);
		processVoice(args, snare2Voice,    SNARE2TRIGIN_INPUT,    SNARE2PUSH_PARAM,    speed, lengthRatio, loopEnabled);
		processVoice(args, closedHatVoice, CLOSEDHATTRIGIN_INPUT, CLOSEDHATPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, openHatVoice,   OPENHATTRIGIN_INPUT,   OPENHATPUSH_PARAM,   speed, lengthRatio, loopEnabled);
		processVoice(args, bongo1Voice,    BONGO1TRIGIN_INPUT,    BONGO1PUSH_PARAM,    speed, lengthRatio, loopEnabled);
		processVoice(args, bongo2Voice,    BONGO2TRIGIN_INPUT,    BONGO2PUSH_PARAM,    speed, lengthRatio, loopEnabled);
		processVoice(args, bongo3Voice,    BONGO3TRIGIN_INPUT,    BONGO3PUSH_PARAM,    speed, lengthRatio, loopEnabled);
		processVoice(args, claveVoice,     CLAVETRIGIN_INPUT,     CLAVEPUSH_PARAM,     speed, lengthRatio, loopEnabled);
		processVoice(args, cymbalVoice,    CYMTRIGIN_INPUT,       CYMPUSH_PARAM,       speed, lengthRatio, loopEnabled);
	}
};

struct AyysKingWidget : ModuleWidget {
	AyysKingWidget(AyysKing* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/AyysKing_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 21.308)), module, AyysKing::SPEED__PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 33.508)), module, AyysKing::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(41.683, 33.508)), module, AyysKing::LOOP_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 59.14)), module, AyysKing::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 59.14)), module, AyysKing::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 59.14)), module, AyysKing::SNARE1PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 59.14)), module, AyysKing::SNARE1_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 59.14)), module, AyysKing::SNARE2PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 59.14)), module, AyysKing::SNARE2_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 59.14)), module, AyysKing::CLOSEDHATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 59.14)), module, AyysKing::CLOSEDHAT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 59.14)), module, AyysKing::OPENHATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 59.14)), module, AyysKing::OPENHAT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 94.743)), module, AyysKing::BONGO1PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 94.743)), module, AyysKing::BONGO1_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 94.743)), module, AyysKing::BONGO2PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 94.743)), module, AyysKing::BONGO2_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 94.743)), module, AyysKing::BONGO3PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 94.743)), module, AyysKing::BONGO3_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 94.743)), module, AyysKing::CLAVEPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 94.743)), module, AyysKing::CLAVE_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 94.743)), module, AyysKing::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 94.743)), module, AyysKing::CYMBAL_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 33.97)), module, AyysKing::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 45.349)), module, AyysKing::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.683, 45.349)), module, AyysKing::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 71.357)), module, AyysKing::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 71.357)), module, AyysKing::SNARE1TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 71.357)), module, AyysKing::SNARE2TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 71.357)), module, AyysKing::CLOSEDHATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 71.357)), module, AyysKing::OPENHATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 106.959)), module, AyysKing::BONGO1TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 106.959)), module, AyysKing::BONGO2TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 106.959)), module, AyysKing::BONGO3TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 106.959)), module, AyysKing::CLAVETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 106.959)), module, AyysKing::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 83.813)), module, AyysKing::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 83.813)), module, AyysKing::SNARE1OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 83.813)), module, AyysKing::SNARE2OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 83.813)), module, AyysKing::CLOSEDHATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 83.813)), module, AyysKing::OPENHATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 118.357)), module, AyysKing::BONGO1OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 118.357)), module, AyysKing::BONGO2OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 118.357)), module, AyysKing::BONGO3OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 118.357)), module, AyysKing::CLAVEOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 118.357)), module, AyysKing::CYMOUT_OUTPUT));
	}
};


Model* modelAyysKing = createModel<AyysKing, AyysKingWidget>("AyysKing");