#include "plugin.hpp"
#include "SeaArrSamples.hpp"

struct SpeedQuantity : ParamQuantity {
	std::string getDisplayValueString() override {
		float v = getValue();
		float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
		return string::f("%.3gx", display);
	}
};

struct SeaArr : Module {
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		MAINVOL_PARAM,
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
		SUM_OUTPUT,
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

		configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configParam(MAINVOL_PARAM, 0.f, 1.f, 1.f, "Main Volume", "%", 0.f, 100.f);

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
		configOutput(SUM_OUTPUT, "Sum");

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
		v.sampleLength = size / 2;
		v.outputId = outputId;
		v.lightId = lightId;
		return v;
	}

	void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId, float speed, float lengthRatio) {
		bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
		bool buttonTrigger = params[pushParamId].getValue() > 0.5f;

		bool inputRising = inputTrigger && !voice.lastInputTrigger;
		bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;

		bool fired = inputRising || buttonRising;

		if (fired) {
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
				outputs[voice.outputId].setVoltage(sample * 10.f);
				voice.samplePos += speed;
			} else {
				voice.playing = false;
				outputs[voice.outputId].setVoltage(0.f);
			}
		} else {
			outputs[voice.outputId].setVoltage(0.f);
		}

		if (voice.lightId >= 0) {
			if (fired)
				lights[voice.lightId].setBrightness(1.f);
			else
				lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
		}
	}

	void process(const ProcessArgs& args) override {
		// --- Precalculate speed with CV ---
		float knobSpeedV = (params[SPEED_PARAM].getValue() + 1.f) * 2.5f;
		float speedCVV = 0.f;
		if (inputs[SPEEDCVIN_INPUT].isConnected())
			speedCVV = std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f;
		float normSpeed = std::clamp(knobSpeedV + speedCVV, 0.f, 5.f) / 5.f;
		float speed = SPEED_MIN + normSpeed * (SPEED_MAX - SPEED_MIN);

		// --- Length ---
		float knobLength = params[LENGTH_PARAM].getValue() * 5.f;
		float lengthCV = inputs[LENGTHCVIN_INPUT].isConnected() ? inputs[LENGTHCVIN_INPUT].getVoltage() : 0.f;
		lengthCV = std::clamp(lengthCV * 0.5f, -5.f, 5.f);
		float combined = knobLength + lengthCV;
		float normLength = std::clamp(combined / 5.f, 0.f, 1.f);
		float lengthRatio = LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);

		processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed, lengthRatio);
		processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed, lengthRatio);
		processVoice(args, hatVoice, HATTRIGIN_INPUT, HATPUSH_PARAM, speed, lengthRatio);
		processVoice(args, hatMetalVoice, HATMETALTRIGIN_INPUT, HATMETALPUSH_PARAM, speed, lengthRatio);
		processVoice(args, rimVoice, RIMTRIGIN_INPUT, RIMPUSH_PARAM, speed, lengthRatio);
		processVoice(args, cowVoice, COWTRIGIN_INPUT, COWPUSH_PARAM, speed, lengthRatio);
		processVoice(args, congaVoice, CONGATRIGIN_INPUT, CONGAPUSH_PARAM, speed, lengthRatio);
		processVoice(args, bongoLVoice, BONGOLTRIGIN_INPUT, BONGOLPUSH_PARAM, speed, lengthRatio);
		processVoice(args, bongoHVoice, BONGOHTRIGIN_INPUT, BONGOHPUSH_PARAM, speed, lengthRatio);
		processVoice(args, tambVoice, TAMTRIGIN_INPUT, TAMPUSH_PARAM, speed, lengthRatio);
		processVoice(args, guiroVoice, GUIROTRIGIN_INPUT, GUIROPUSH_PARAM, speed, lengthRatio);
		processVoice(args, cymVoice, CYMTRIGIN_INPUT, CYMPUSH_PARAM, speed, lengthRatio);

		// --- Sum output ---
		float mainVol = params[MAINVOL_PARAM].getValue();
		float busSum = 0.f;
		int busCount = 0;
		Voice* allVoices[] = {
			&kickVoice, &snareVoice, &hatVoice, &hatMetalVoice, &rimVoice,
			&cowVoice, &congaVoice, &bongoLVoice, &bongoHVoice, &tambVoice, &guiroVoice, &cymVoice
		};
		for (Voice* v : allVoices) {
			if (!outputs[v->outputId].isConnected()) {
				busSum += outputs[v->outputId].getVoltage();
				busCount++;
			}
		}
		float sumOut = busCount > 0 ? (busSum / busCount) * mainVol : 0.f;
		outputs[SUM_OUTPUT].setVoltage(std::clamp(sumOut, -5.f, 5.f));
	}
};

struct SeaArrWidget : ModuleWidget {
	SeaArrWidget(SeaArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SeaArr.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(16.799, 15.501)), module, SeaArr::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(38.799, 15.501)), module, SeaArr::SPEED_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(59.351, 15.501)), module, SeaArr::MAINVOL_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(8.798, 42.002)), module, SeaArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(8.798, 42.002)), module, SeaArr::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(20.5, 42.002)), module, SeaArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(20.5, 42.002)), module, SeaArr::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(32.3, 42.002)), module, SeaArr::HATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(32.3, 42.002)), module, SeaArr::HAT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(43.998, 42.002)), module, SeaArr::HATMETALPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(43.998, 42.002)), module, SeaArr::HATMETAL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(55.7, 42.002)), module, SeaArr::RIMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(55.7, 42.002)), module, SeaArr::RIM_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(67.501, 42.002)), module, SeaArr::COWPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(67.501, 42.002)), module, SeaArr::COW_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(8.798, 84.999)), module, SeaArr::CONGAPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(8.798, 84.999)), module, SeaArr::CONGA_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(20.5, 84.999)), module, SeaArr::BONGOLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(20.5, 84.999)), module, SeaArr::BONGOL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(32.3, 84.999)), module, SeaArr::BONGOHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(32.3, 84.999)), module, SeaArr::BONGOH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(43.998, 84.999)), module, SeaArr::TAMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(43.998, 84.999)), module, SeaArr::TAM_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(55.7, 84.999)), module, SeaArr::GUIROPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(55.7, 84.999)), module, SeaArr::GUIRO_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(67.501, 84.999)), module, SeaArr::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(67.501, 84.999)), module, SeaArr::CYM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.849, 30.801)), module, SeaArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.851, 30.801)), module, SeaArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.798, 56.0)), module, SeaArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.5, 56.0)), module, SeaArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.3, 56.0)), module, SeaArr::HATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.998, 56.0)), module, SeaArr::HATMETALTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.7, 56.0)), module, SeaArr::RIMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67.501, 56.0)), module, SeaArr::COWTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.798, 99.001)), module, SeaArr::CONGATRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.5, 99.001)), module, SeaArr::BONGOLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.3, 99.001)), module, SeaArr::BONGOHTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.998, 99.001)), module, SeaArr::TAMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.7, 99.001)), module, SeaArr::GUIROTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67.501, 99.001)), module, SeaArr::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(59.351, 30.801)), module, SeaArr::SUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.798, 70.002)), module, SeaArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.5, 70.002)), module, SeaArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.3, 70.002)), module, SeaArr::HATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 70.002)), module, SeaArr::HATMETALOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.7, 70.002)), module, SeaArr::RIMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(67.501, 70.002)), module, SeaArr::COWOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.798, 112.999)), module, SeaArr::CONGAOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.5, 112.999)), module, SeaArr::BONGOLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.3, 112.999)), module, SeaArr::BONGOHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.999)), module, SeaArr::TAMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.7, 112.999)), module, SeaArr::GUIROOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(67.501, 112.999)), module, SeaArr::CYMOUT_OUTPUT));
	}
};


Model* modelSeaArr = createModel<SeaArr, SeaArrWidget>("SeaArr");
