#include "plugin.hpp"
#include "KayArrSamples.hpp"

struct KayArr : Module {
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNAREPUSH_PARAM,
		TOMPUSH_PARAM,
		CLPUSH_PARAM,
		OHPUSH_PARAM,
		CLAVEPUSH_PARAM,
		RIMSHOTPUSH_PARAM,
		COWBELLPUSH_PARAM,
		CYMPUSH_PARAM,
		CONGAPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT,
		SNARETRIGIN_INPUT,
		TOMTRIG_INPUT,
		CLTRIG_INPUT,
		OHTRIG_INPUT,
		CLAVETRIG_INPUT,
		RIMSHOTTRIG_INPUT,
		COWBELLTRIG_INPUT,
		CYMBALTRIG_INPUT,
		CONGATRIG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		TOMOUT_OUTPUT,
		CLOUT_OUTPUT,
		OHOUT_OUTPUT,
		CLAVEOUT_OUTPUT,
		RIMSHOTOUT_OUTPUT,
		COWBELLOUT_OUTPUT,
		CYMBALOUT_OUTPUT,
		CONGAOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE_LIGHT,
		TOM_LIGHT,
		CL_LIGHT,
		OH_LIGHT,
		CLAVE_LIGHT,
		RIMSHOT_LIGHT,
		COWBELL_LIGHT,
		CYMBAL_LIGHT,
		CONGA_LIGHT,
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
	

	Voice kickVoice, snareVoice, tomVoice;
	Voice closedHatVoice, openHatVoice;
	Voice claveVoice, rimshotVoice, cowbellVoice, cymbalVoice, congaVoice;

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 3.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	KayArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SPEED_PARAM, 0.f, 1.f, 0.33f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(TOMPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Trig", {"Off", "On"});
		configSwitch(CLPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OHPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});
		configSwitch(CLAVEPUSH_PARAM, 0.f, 1.f, 0.f, "Clave Trig", {"Off", "On"});
		configSwitch(RIMSHOTPUSH_PARAM, 0.f, 1.f, 0.f, "Rimshot Trig", {"Off", "On"});
		configSwitch(COWBELLPUSH_PARAM, 0.f, 1.f, 0.f, "Cowbell Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});
		configSwitch(CONGAPUSH_PARAM, 0.f, 1.f, 0.f, "Conga Trig", {"Off", "On"});

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop Gate");

		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(TOMTRIG_INPUT, "Tom Trig");
		configInput(CLTRIG_INPUT, "Closed Hat Trig");
		configInput(OHTRIG_INPUT, "Open Hat Trig");
		configInput(CLAVETRIG_INPUT, "Clave Trig");
		configInput(RIMSHOTTRIG_INPUT, "Rimshot Trig");
		configInput(COWBELLTRIG_INPUT, "Cowbell Trig");
		configInput(CYMBALTRIG_INPUT, "Cymbal Trig");
		configInput(CONGATRIG_INPUT, "Conga Trig");

		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(TOMOUT_OUTPUT, "Tom");
		configOutput(CLOUT_OUTPUT, "Closed Hat");
		configOutput(OHOUT_OUTPUT, "Open Hat");
		configOutput(CLAVEOUT_OUTPUT, "Clave");
		configOutput(RIMSHOTOUT_OUTPUT, "Rimshot");
		configOutput(COWBELLOUT_OUTPUT, "Cowbell");
		configOutput(CYMBALOUT_OUTPUT, "Cymbal");
		configOutput(CONGAOUT_OUTPUT, "Conga");

		kickVoice      = createVoice(KRKick, sizeof(KRKick), KICKOUT_OUTPUT, KICK_LIGHT);
		snareVoice     = createVoice(KRSnare, sizeof(KRSnare), SNAREOUT_OUTPUT, SNARE_LIGHT);
		tomVoice       = createVoice(KRTom, sizeof(KRTom), TOMOUT_OUTPUT, TOM_LIGHT);
		closedHatVoice = createVoice(KRClosedHat, sizeof(KRClosedHat), CLOUT_OUTPUT, CL_LIGHT);
		openHatVoice   = createVoice(KROpenHat, sizeof(KROpenHat), OHOUT_OUTPUT, OH_LIGHT);
		claveVoice     = createVoice(KRClave, sizeof(KRClave), CLAVEOUT_OUTPUT, CLAVE_LIGHT);
		rimshotVoice   = createVoice(KRRimshot, sizeof(KRRimshot), RIMSHOTOUT_OUTPUT, RIMSHOT_LIGHT);
		cowbellVoice   = createVoice(KRCowbell, sizeof(KRCowbell), COWBELLOUT_OUTPUT, COWBELL_LIGHT);
		cymbalVoice    = createVoice(KRCymbal, sizeof(KRCymbal), CYMBALOUT_OUTPUT, CYMBAL_LIGHT);
		congaVoice     = createVoice(KRConga, sizeof(KRConga), CONGAOUT_OUTPUT, CONGA_LIGHT);
		
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
		float loopCV = inputs[LOOPCVIN_INPUT].getVoltage();
		bool loopEnabled = baseLoop || (!baseLoop && loopCV > 1.f);

		processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, tomVoice, TOMTRIG_INPUT, TOMPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, closedHatVoice, CLTRIG_INPUT, CLPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, openHatVoice, OHTRIG_INPUT, OHPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, claveVoice, CLAVETRIG_INPUT, CLAVEPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, rimshotVoice, RIMSHOTTRIG_INPUT, RIMSHOTPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, cowbellVoice, COWBELLTRIG_INPUT, COWBELLPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, cymbalVoice, CYMBALTRIG_INPUT, CYMPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, congaVoice, CONGATRIG_INPUT, CONGAPUSH_PARAM, speed, lengthRatio, loopEnabled);
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
struct KayArrWidget : ModuleWidget {
	KayArrWidget(KayArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/KayArr_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 21.308)), module, KayArr::SPEED_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 33.508)), module, KayArr::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(41.683, 33.508)), module, KayArr::LOOP_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 59.14)), module, KayArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 59.14)), module, KayArr::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 59.14)), module, KayArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 59.14)), module, KayArr::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 59.14)), module, KayArr::CLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 59.14)), module, KayArr::CL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 59.14)), module, KayArr::OHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 59.14)), module, KayArr::OH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 59.14)), module, KayArr::TOMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 59.14)), module, KayArr::TOM_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 94.743)), module, KayArr::CONGAPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 94.743)), module, KayArr::CONGA_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 94.743)), module, KayArr::CLAVEPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 94.743)), module, KayArr::CLAVE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 94.743)), module, KayArr::RIMSHOTPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 94.743)), module, KayArr::RIMSHOT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 94.743)), module, KayArr::COWBELLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 94.743)), module, KayArr::COWBELL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 94.743)), module, KayArr::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 94.743)), module, KayArr::CYMBAL_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 33.97)), module, KayArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 45.349)), module, KayArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.683, 45.349)), module, KayArr::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 71.357)), module, KayArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 71.357)), module, KayArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 71.357)), module, KayArr::CLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 71.357)), module, KayArr::OHTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 71.357)), module, KayArr::TOMTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 106.959)), module, KayArr::CONGATRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 106.959)), module, KayArr::CLAVETRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 106.959)), module, KayArr::RIMSHOTTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 106.959)), module, KayArr::COWBELLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 106.959)), module, KayArr::CYMBALTRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 83.813)), module, KayArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 83.813)), module, KayArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 83.813)), module, KayArr::CLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 83.813)), module, KayArr::OHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 83.813)), module, KayArr::TOMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 118.357)), module, KayArr::CONGAOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 118.357)), module, KayArr::CLAVEOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 118.357)), module, KayArr::RIMSHOTOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 118.357)), module, KayArr::COWBELLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 118.357)), module, KayArr::CYMBALOUT_OUTPUT));
	}
};


Model* modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");