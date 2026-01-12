#include "plugin.hpp"
#include "SeaArrSamples.hpp"  // Your samples header

struct SeaArr : Module {
	enum ParamId {
		SPEED__PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNAREPUSH_PARAM,
		HATPUSH_PARAM,
		HATMETALPUSH_PARAM,
		RIMPUSH_PARAM,
		COWPUSH_PARAM,
		CONGAPUSH_PARAM,
		BONGOLPUSH_PARAM,
		BONGOHPUSH_PARAM,
		TAMPUSH_PARAM,
		GUIROPUSH_PARAM,
		CYMPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT,
		SNARETRIGIN_INPUT,
		HATTRIGIN_INPUT,
		HATMETALTRIGIN_INPUT,
		RIMTRIGIN_INPUT,
		COWTRIGIN_INPUT,
		CONGATRIGIN_INPUT,
		BONGOLTRIGIN_INPUT,
		BONGOHTRIGIN_INPUT,
		TAMTRIGIN_INPUT,
		GUIROTRIGIN_INPUT,
		CYMTRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		HATOUT_OUTPUT,
		HATMETALOUT_OUTPUT,
		RIMOUT_OUTPUT,
		COWOUT_OUTPUT,
		CONGAOUT_OUTPUT,
		BONGOLOUT_OUTPUT,
		BONGOHOUT_OUTPUT,
		TAMOUT_OUTPUT,
		GUIROOUT_OUTPUT,
		CYMOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE_LIGHT,
		HAT_LIGHT,
		HATMETAL_LIGHT,
		RIM_LIGHT,
		COW_LIGHT,
		CONGA_LIGHT,
		BONGOL_LIGHT,
		BONGOH_LIGHT,
		TAM_LIGHT,
		GUIRO_LIGHT,
		CYM_LIGHT,
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

	Voice kickVoice, snareVoice, hatVoice, hatMetalVoice, rimVoice;
	Voice cowVoice, congaVoice, bongoLVoice, bongoHVoice, tambVoice, guiroVoice, cymVoice;

	const float SPEED_MIN = 0.05f;
	const float SPEED_MAX = 2.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	SeaArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SPEED__PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(HATPUSH_PARAM, 0.f, 1.f, 0.f, "Hat Trig", {"Off", "On"});
		configSwitch(HATMETALPUSH_PARAM, 0.f, 1.f, 0.f, "Hat Metal Trig", {"Off", "On"});
		configSwitch(RIMPUSH_PARAM, 0.f, 1.f, 0.f, "Rimshot Trig", {"Off", "On"});
		configSwitch(COWPUSH_PARAM, 0.f, 1.f, 0.f, "Cowbell Trig", {"Off", "On"});
		configSwitch(CONGAPUSH_PARAM, 0.f, 1.f, 0.f, "Conga Trig", {"Off", "On"});
		configSwitch(BONGOLPUSH_PARAM, 0.f, 1.f, 0.f, "Bongo Lo Trig", {"Off", "On"});
		configSwitch(BONGOHPUSH_PARAM, 0.f, 1.f, 0.f, "Bongo Hi Trig", {"Off", "On"});
		configSwitch(TAMPUSH_PARAM, 0.f, 1.f, 0.f, "Tambourine Trig", {"Off", "On"});
		configSwitch(GUIROPUSH_PARAM, 0.f, 1.f, 0.f, "Guiro Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(HATTRIGIN_INPUT, "Hi-Hat Trig");
		configInput(HATMETALTRIGIN_INPUT, "Hi-Hat Metal Trig");
		configInput(RIMTRIGIN_INPUT, "Rimshot Trig");
		configInput(COWTRIGIN_INPUT, "Cowbell Trig");
		configInput(CONGATRIGIN_INPUT, "Conga Trig");
		configInput(BONGOLTRIGIN_INPUT, "Bongo Lo Trig");
		configInput(BONGOHTRIGIN_INPUT, "Bongo Hi Trig");
		configInput(TAMTRIGIN_INPUT, "Tambourine Trig");
		configInput(GUIROTRIGIN_INPUT, "Guiro Trig");
		configInput(CYMTRIGIN_INPUT, "Cymbal Trig");

		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(HATOUT_OUTPUT, "Hi-Hat");
		configOutput(HATMETALOUT_OUTPUT, "Hi-Hat Metal");
		configOutput(RIMOUT_OUTPUT, "Rimshot");
		configOutput(COWOUT_OUTPUT, "Cowbell");
		configOutput(CONGAOUT_OUTPUT, "Conga");
		configOutput(BONGOLOUT_OUTPUT, "Bongo Lo");
		configOutput(BONGOHOUT_OUTPUT, "Bongo Hi");
		configOutput(TAMOUT_OUTPUT, "Tambourine");
		configOutput(GUIROOUT_OUTPUT, "Guiro");
		configOutput(CYMOUT_OUTPUT, "Cymbal");

		// Initialize voices with sample data from SeaArrSamples.hpp
		kickVoice = createVoice(SAKick, sizeof(SAKick), KICKOUT_OUTPUT, KICK_LIGHT);
		snareVoice = createVoice(SASnare, sizeof(SASnare), SNAREOUT_OUTPUT, SNARE_LIGHT);
		hatVoice = createVoice(SAHiHat, sizeof(SAHiHat), HATOUT_OUTPUT, HAT_LIGHT);
		hatMetalVoice = createVoice(SAHiHatMetal, sizeof(SAHiHatMetal), HATMETALOUT_OUTPUT, HATMETAL_LIGHT);
		rimVoice = createVoice(SARim, sizeof(SARim), RIMOUT_OUTPUT, RIM_LIGHT);
		cowVoice = createVoice(SACowbell, sizeof(SACowbell), COWOUT_OUTPUT, COW_LIGHT);
		congaVoice = createVoice(SACongaL, sizeof(SACongaL), CONGAOUT_OUTPUT, CONGA_LIGHT);
		bongoLVoice = createVoice(SABongoL, sizeof(SABongoL), BONGOLOUT_OUTPUT, BONGOL_LIGHT);
		bongoHVoice = createVoice(SABongoH, sizeof(SABongoH), BONGOHOUT_OUTPUT, BONGOH_LIGHT);
		tambVoice = createVoice(SATamb, sizeof(SATamb), TAMOUT_OUTPUT, TAM_LIGHT);
		guiroVoice = createVoice(SAGuiro, sizeof(SAGuiro), GUIROOUT_OUTPUT, GUIRO_LIGHT);
		cymVoice = createVoice(SACym, sizeof(SACym), CYMOUT_OUTPUT, CYM_LIGHT);
	}

	Voice createVoice(const unsigned char* data, size_t size, int outputId, int lightId) {
		Voice v;
		v.rawData = data;
		v.sampleLength = size / 2; // 16-bit samples
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
		float speed = rescale(params[SPEED__PARAM].getValue(), 0.f, 1.f, SPEED_MIN, SPEED_MAX);
		if (inputs[SPEEDCVIN_INPUT].isConnected()) {
			speed *= std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage() / 10.f, 0.f, 1.f);
		}

		float lengthRatio = rescale(params[LENGTH_PARAM].getValue(), 0.f, 1.f, LENGTH_MIN, LENGTH_MAX);
		if (inputs[LENGTHCVIN_INPUT].isConnected()) {
			lengthRatio *= std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage() / 10.f, 0.f, 1.f);
		}

		bool loopEnabled = (params[LOOP_PARAM].getValue() > 0.5f) || (inputs[LOOPCVIN_INPUT].getVoltage() > 1.f);

		processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, hatVoice, HATTRIGIN_INPUT, HATPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, hatMetalVoice, HATMETALTRIGIN_INPUT, HATMETALPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, rimVoice, RIMTRIGIN_INPUT, RIMPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, cowVoice, COWTRIGIN_INPUT, COWPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, congaVoice, CONGATRIGIN_INPUT, CONGAPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, bongoLVoice, BONGOLTRIGIN_INPUT, BONGOLPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, bongoHVoice, BONGOHTRIGIN_INPUT, BONGOHPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, tambVoice, TAMTRIGIN_INPUT, TAMPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, guiroVoice, GUIROTRIGIN_INPUT, GUIROPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, cymVoice, CYMTRIGIN_INPUT, CYMPUSH_PARAM, speed, lengthRatio, loopEnabled);
	}
};

struct SeaArrWidget : ModuleWidget {
	SeaArrWidget(SeaArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SeaArr_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(30.48, 21.308)), module, SeaArr::SPEED__PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(16.002, 33.508)), module, SeaArr::LENGTH_PARAM));
		addParam(createParamCentered<Switch2Pos>(mm2px(Vec(46.446, 33.508)), module, SeaArr::LOOP_PARAM));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 59.14)), module, SeaArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 59.14)), module, SeaArr::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 59.14)), module, SeaArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 59.14)), module, SeaArr::SNARE_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 59.14)), module, SeaArr::HATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 59.14)), module, SeaArr::HAT_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 59.14)), module, SeaArr::HATMETALPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 59.14)), module, SeaArr::HATMETAL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 59.14)), module, SeaArr::RIMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 59.14)), module, SeaArr::RIM_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.519, 59.14)), module, SeaArr::COWPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(54.519, 59.14)), module, SeaArr::COW_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 94.743)), module, SeaArr::CONGAPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 94.743)), module, SeaArr::CONGA_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 94.743)), module, SeaArr::BONGOLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 94.743)), module, SeaArr::BONGOL_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 94.743)), module, SeaArr::BONGOHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 94.743)), module, SeaArr::BONGOH_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 94.743)), module, SeaArr::TAMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 94.743)), module, SeaArr::TAM_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 94.743)), module, SeaArr::GUIROPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 94.743)), module, SeaArr::GUIRO_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.519, 94.743)), module, SeaArr::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(54.519, 94.743)), module, SeaArr::CYM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 33.97)), module, SeaArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.002, 45.349)), module, SeaArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.446, 45.349)), module, SeaArr::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 71.357)), module, SeaArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 71.357)), module, SeaArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 71.357)), module, SeaArr::HATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 71.357)), module, SeaArr::HATMETALTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 71.357)), module, SeaArr::RIMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.519, 71.357)), module, SeaArr::COWTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 106.959)), module, SeaArr::CONGATRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 106.959)), module, SeaArr::BONGOLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 106.959)), module, SeaArr::BONGOHTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 106.959)), module, SeaArr::TAMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 106.959)), module, SeaArr::GUIROTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.519, 106.959)), module, SeaArr::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 83.813)), module, SeaArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 83.813)), module, SeaArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 83.813)), module, SeaArr::HATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 83.813)), module, SeaArr::HATMETALOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 83.813)), module, SeaArr::RIMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.519, 83.813)), module, SeaArr::COWOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 118.357)), module, SeaArr::CONGAOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 118.357)), module, SeaArr::BONGOLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 118.357)), module, SeaArr::BONGOHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 118.357)), module, SeaArr::TAMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 118.357)), module, SeaArr::GUIROOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.519, 118.357)), module, SeaArr::CYMOUT_OUTPUT));
	}
};


Model* modelSeaArr = createModel<SeaArr, SeaArrWidget>("SeaArr");