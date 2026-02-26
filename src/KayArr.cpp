#include "plugin.hpp"
#include "KayArrSamples.hpp"

struct KayArr : Module {
	enum ParamId {
		SPEED_PARAM, LENGTH_PARAM, LOOP_PARAM,
		KICKPUSH_PARAM, SNAREPUSH_PARAM, TOMPUSH_PARAM,
		CLPUSH_PARAM, OHPUSH_PARAM, CLAVEPUSH_PARAM,
		RIMSHOTPUSH_PARAM, COWBELLPUSH_PARAM, CYMPUSH_PARAM,
		CONGAPUSH_PARAM, PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT, LENGTHCVIN_INPUT, LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT, SNARETRIGIN_INPUT, TOMTRIG_INPUT,
		CLTRIG_INPUT, OHTRIG_INPUT, CLAVETRIG_INPUT,
		RIMSHOTTRIG_INPUT, COWBELLTRIG_INPUT, CYMBALTRIG_INPUT,
		CONGATRIG_INPUT, INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT, SNAREOUT_OUTPUT, TOMOUT_OUTPUT,
		CLOUT_OUTPUT, OHOUT_OUTPUT, CLAVEOUT_OUTPUT,
		RIMSHOTOUT_OUTPUT, COWBELLOUT_OUTPUT, CYMBALOUT_OUTPUT,
		CONGAOUT_OUTPUT, OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT, SNARE_LIGHT, TOM_LIGHT,
		CL_LIGHT, OH_LIGHT, CLAVE_LIGHT,
		RIMSHOT_LIGHT, COWBELL_LIGHT, CYMBAL_LIGHT,
		CONGA_LIGHT, LOOP_LIGHT, LIGHTS_LEN
	};

	struct Voice {
		const uint8_t* rawData = nullptr;
		int sampleLength = 0;
		int outputId = -1;
		int lightId = -1;
		int trigInputId = -1;
		int pushParamId = -1;

		bool lastInputTrigger = false;
		bool lastButtonTrigger = false;
		float samplePos = 0.f;
		bool playing = false;

		inline int16_t readSample16(int index) {
			return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
		}
	};

	Voice voices[10];

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 3.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	KayArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SPEED_PARAM,  0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f,  "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM,  0.f, 1.f, 0.f,  "Loop", {"Off", "On"});

		configSwitch(KICKPUSH_PARAM,     0.f, 1.f, 0.f, "Kick Trig",     {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM,    0.f, 1.f, 0.f, "Snare Trig",    {"Off", "On"});
		configSwitch(TOMPUSH_PARAM,      0.f, 1.f, 0.f, "Tom Trig",      {"Off", "On"});
		configSwitch(CLPUSH_PARAM,       0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OHPUSH_PARAM,       0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});
		configSwitch(CLAVEPUSH_PARAM,    0.f, 1.f, 0.f, "Clave Trig",    {"Off", "On"});
		configSwitch(RIMSHOTPUSH_PARAM,  0.f, 1.f, 0.f, "Rimshot Trig",  {"Off", "On"});
		configSwitch(COWBELLPUSH_PARAM,  0.f, 1.f, 0.f, "Cowbell Trig",  {"Off", "On"});
		configSwitch(CYMPUSH_PARAM,      0.f, 1.f, 0.f, "Cymbal Trig",   {"Off", "On"});
		configSwitch(CONGAPUSH_PARAM,    0.f, 1.f, 0.f, "Conga Trig",    {"Off", "On"});

		configInput(SPEEDCVIN_INPUT,   "Speed CV");
		configInput(LENGTHCVIN_INPUT,  "Length CV");
		configInput(LOOPCVIN_INPUT,    "Loop Gate");

		configInput(KICKTRIGIN_INPUT,     "Kick Trig");
		configInput(SNARETRIGIN_INPUT,    "Snare Trig");
		configInput(TOMTRIG_INPUT,        "Tom Trig");
		configInput(CLTRIG_INPUT,         "Closed Hat Trig");
		configInput(OHTRIG_INPUT,         "Open Hat Trig");
		configInput(CLAVETRIG_INPUT,      "Clave Trig");
		configInput(RIMSHOTTRIG_INPUT,    "Rimshot Trig");
		configInput(COWBELLTRIG_INPUT,    "Cowbell Trig");
		configInput(CYMBALTRIG_INPUT,     "Cymbal Trig");
		configInput(CONGATRIG_INPUT,      "Conga Trig");

		configOutput(KICKOUT_OUTPUT,     "Kick");
		configOutput(SNAREOUT_OUTPUT,    "Snare");
		configOutput(TOMOUT_OUTPUT,      "Tom");
		configOutput(CLOUT_OUTPUT,       "Closed Hat");
		configOutput(OHOUT_OUTPUT,       "Open Hat");
		configOutput(CLAVEOUT_OUTPUT,    "Clave");
		configOutput(RIMSHOTOUT_OUTPUT,  "Rimshot");
		configOutput(COWBELLOUT_OUTPUT,  "Cowbell");
		configOutput(CYMBALOUT_OUTPUT,   "Cymbal");
		configOutput(CONGAOUT_OUTPUT,    "Conga");

		static const struct {
			const uint8_t* data;
			int dataSize;
			int trigInput;
			int pushParam;
			int output;
			int light;
		} voiceMap[10] = {
			{ KRKick, sizeof(KRKick), KICKTRIGIN_INPUT, KICKPUSH_PARAM, KICKOUT_OUTPUT, KICK_LIGHT },
			{ KRSnare, sizeof(KRSnare), SNARETRIGIN_INPUT, SNAREPUSH_PARAM, SNAREOUT_OUTPUT, SNARE_LIGHT },
			{ KRTom, sizeof(KRTom), TOMTRIG_INPUT, TOMPUSH_PARAM, TOMOUT_OUTPUT, TOM_LIGHT },
			{ KRClosedHat, sizeof(KRClosedHat), CLTRIG_INPUT, CLPUSH_PARAM, CLOUT_OUTPUT, CL_LIGHT },
			{ KROpenHat, sizeof(KROpenHat), OHTRIG_INPUT, OHPUSH_PARAM, OHOUT_OUTPUT, OH_LIGHT },
			{ KRClave, sizeof(KRClave), CLAVETRIG_INPUT, CLAVEPUSH_PARAM, CLAVEOUT_OUTPUT, CLAVE_LIGHT },
			{ KRRimshot, sizeof(KRRimshot), RIMSHOTTRIG_INPUT, RIMSHOTPUSH_PARAM, RIMSHOTOUT_OUTPUT, RIMSHOT_LIGHT },
			{ KRCowbell, sizeof(KRCowbell), COWBELLTRIG_INPUT, COWBELLPUSH_PARAM, COWBELLOUT_OUTPUT, COWBELL_LIGHT },
			{ KRCymbal, sizeof(KRCymbal), CYMBALTRIG_INPUT, CYMPUSH_PARAM, CYMBALOUT_OUTPUT, CYMBAL_LIGHT },
			{ KRConga, sizeof(KRConga), CONGATRIG_INPUT, CONGAPUSH_PARAM, CONGAOUT_OUTPUT, CONGA_LIGHT }
		};

		for (int i = 0; i < 10; i++) {
			voices[i].rawData = voiceMap[i].data;
			voices[i].sampleLength = voiceMap[i].dataSize / 2;
			voices[i].outputId = voiceMap[i].output;
			voices[i].lightId = voiceMap[i].light;
			voices[i].trigInputId = voiceMap[i].trigInput;
			voices[i].pushParamId = voiceMap[i].pushParam;
		}
	}


	bool loopState = false;           // the current loop on/off state
	bool lastLoopButton = false;      // previous frame state for the button
	bool lastLoopCVTrigger = false;   // previous frame state for the CV
	
	void process(const ProcessArgs& args) override {
		// --- Speed parameter ---
		float speedKnobV = params[SPEED_PARAM].getValue() * 5.f; // knob 0-1 → 0-5V
		float speedCVV = inputs[SPEEDCVIN_INPUT].isConnected() ? std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f : 0.f; // -10..10 → -5..5
		float normSpeed = std::clamp(speedKnobV + speedCVV, 0.f, 5.f) / 5.f; // 0-1 normalized
		float speed = SPEED_LOW + normSpeed * (SPEED_HIGH - SPEED_LOW);
	
		// --- Length parameter ---
		float lengthKnobV = params[LENGTH_PARAM].getValue() * 5.f; // knob 0-1 → 0-5V
		float lengthCVV = inputs[LENGTHCVIN_INPUT].isConnected() ? std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f : 0.f; // -10..10 → -5..5
		float normLength = std::clamp(lengthKnobV + lengthCVV, 0.f, 5.f) / 5.f; // 0-1 normalized
		float lengthRatio = LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);
	
		// --- Loop mode ---
		bool loopButton = params[LOOP_PARAM].getValue() > 0.5f;
		float loopCV = inputs[LOOPCVIN_INPUT].isConnected() ? inputs[LOOPCVIN_INPUT].getVoltage() : 0.f;
		bool loopButtonRising = loopButton && !lastLoopButton;
		if (loopButtonRising)
			loopState = !loopState;
		static bool lastGateHigh = false; // keeps track of gate state across calls
		bool gateHigh = loopCV > 0.6f;
		if (gateHigh && !lastGateHigh)
			loopState = !loopState;
		lastGateHigh = gateHigh;

		lastLoopButton = loopButton;

		bool loopEnabled = loopState;
		lights[LOOP_LIGHT].setBrightnessSmooth(loopState, args.sampleTime);
	
		for (int i = 0; i < 10; i++) {
			Voice& v = voices[i];
	
			bool inputTrig = inputs[v.trigInputId].getVoltage() > 1.f;
			bool buttonTrig = params[v.pushParamId].getValue() > 0.5f;
			bool inputRising = inputTrig && !v.lastInputTrigger;
			bool buttonRising = buttonTrig && !v.lastButtonTrigger;
	
			bool retrigger = inputRising || buttonRising || (loopEnabled && !v.playing);
	
			v.lastInputTrigger = inputTrig;
			v.lastButtonTrigger = buttonTrig;
	
			const int maxSamples = (int)(v.sampleLength * lengthRatio);
	
			bool fired = retrigger; // 🔴 new: track for light
	
			if (retrigger) {
				v.samplePos = 0.f;
				v.playing = true;
			}
	
			if (v.playing) {
				int idx = (int)v.samplePos;
				if (idx < maxSamples) {
					float sample = v.readSample16(idx) / 32768.f;
					outputs[v.outputId].setVoltage(sample * 10.f);
					v.samplePos += speed;
				} else {
					if (loopEnabled) {
						v.samplePos = 0.f;
						fired = true; // 🔴 loop restart fires light
					} else {
						v.playing = false;
						outputs[v.outputId].setVoltage(0.f);
					}
				}
			} else {
				outputs[v.outputId].setVoltage(0.f);
			}
	
			// Efficient light fade
			if (v.lightId >= 0) {
				if (fired)
					lights[v.lightId].setBrightness(1.f);
				else
					lights[v.lightId].setBrightnessSmooth(0.f, args.sampleTime);
			}
		}
	}
};
	

struct KayArrWidget : ModuleWidget {
	KayArrWidget(KayArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/KayArr.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 15.501)), module, KayArr::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(31.249, 15.501)), module, KayArr::SPEED_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(49.731, 15.501)), module, KayArr::LOOP_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(49.731, 15.501)), module, KayArr::LOOP_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(6.999, 42.002)), module, KayArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(6.999, 42.002)), module, KayArr::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(18.75, 42.002)), module, KayArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(18.75, 42.002)), module, KayArr::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(30.501, 42.002)), module, KayArr::CLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(30.501, 42.002)), module, KayArr::CL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(42.249, 42.002)), module, KayArr::OHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(42.249, 42.002)), module, KayArr::OH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.0, 42.002)), module, KayArr::TOMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(54.0, 42.002)), module, KayArr::TOM_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(6.999, 84.999)), module, KayArr::CONGAPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(6.999, 84.999)), module, KayArr::CONGA_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(18.75, 84.999)), module, KayArr::CLAVEPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(18.75, 84.999)), module, KayArr::CLAVE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(30.501, 84.999)), module, KayArr::RIMSHOTPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(30.501, 84.999)), module, KayArr::RIMSHOT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(42.249, 84.999)), module, KayArr::COWBELLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(42.249, 84.999)), module, KayArr::COWBELL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.0, 84.999)), module, KayArr::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(54.0, 84.999)), module, KayArr::CYMBAL_LIGHT));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 30.801)), module, KayArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.249, 30.801)), module, KayArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.731, 30.801)), module, KayArr::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.999, 56.0)), module, KayArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.75, 56.0)), module, KayArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.501, 56.0)), module, KayArr::CLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.249, 56.0)), module, KayArr::OHTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.0, 56.0)), module, KayArr::TOMTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.999, 99.001)), module, KayArr::CONGATRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.75, 99.001)), module, KayArr::CLAVETRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.501, 99.001)), module, KayArr::RIMSHOTTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.249, 99.001)), module, KayArr::COWBELLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.0, 99.001)), module, KayArr::CYMBALTRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.999, 70.002)), module, KayArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.75, 70.002)), module, KayArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.501, 70.002)), module, KayArr::CLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.249, 70.002)), module, KayArr::OHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.0, 70.002)), module, KayArr::TOMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.999, 112.999)), module, KayArr::CONGAOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.75, 112.999)), module, KayArr::CLAVEOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.501, 112.999)), module, KayArr::RIMSHOTOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.249, 112.999)), module, KayArr::COWBELLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.0, 112.999)), module, KayArr::CYMBALOUT_OUTPUT));
	}
};


Model* modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");
