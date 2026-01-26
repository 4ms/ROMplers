#include "plugin.hpp"
#include "DeeArrSamples.hpp" // Make sure this provides raw byte arrays DAKick, DASnare, DAHat, DARim

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
		LOOP_LIGHT,
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
		int outputId = 0;
		int lightId = -1;

		// Pre-decoded float samples
		std::vector<float> decodedSample;

		// Output zero flag to avoid redundant zero output writes
		bool outputZeroSet = true;

		void loadSample(const unsigned char* data, int lengthBytes) {
			int sampleCount = lengthBytes / 2;
			decodedSample.resize(sampleCount);
			for (int i = 0; i < sampleCount; ++i) {
				int16_t s = (int16_t)(data[2*i] | (data[2*i + 1] << 8));
				decodedSample[i] = s / 32768.f;
			}
			samplePos = 0.f;
			playing = false;
			outputZeroSet = true;
		}
	};

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 2.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	Voice kickVoice, snareVoice, hatVoice, rimVoice;

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

		// Preload samples once, decoding into float vectors
		kickVoice = createVoice(DAKick, sizeof(DAKick), KICKOUT_OUTPUT, KICK_LIGHT);
		snareVoice = createVoice(DASnare, sizeof(DASnare), SNAREOUT_OUTPUT, SNARE_LIGHT);
		hatVoice = createVoice(DAHat, sizeof(DAHat), HATOUT_OUTPUT, HAT_LIGHT);
		rimVoice = createVoice(DARim, sizeof(DARim), RIMOUT_OUTPUT, RIM_LIGHT);
	}

	Voice createVoice(const unsigned char* data, size_t size, int outputId, int lightId) {
		Voice v;
		v.outputId = outputId;
		v.lightId = lightId;
		v.loadSample(data, (int)size);
		return v;
	}

	bool loopState = false;           // the current loop on/off state
	bool lastLoopButton = false;      // previous frame state for the button
	bool lastLoopCVTrigger = false;   // previous frame state for the CV

	void process(const ProcessArgs& args) override {
		// --- Precalculate speed with CV ---
		float knobSpeedV = params[SPEED_PARAM].getValue() * 5.f;  // 0–1 → 0–5V
		float speedCVV = 0.f;
		if (inputs[SPEEDCVIN_INPUT].isConnected())
			speedCVV = std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f; // -10..10 → -5..5
		float normSpeed = std::clamp(knobSpeedV + speedCVV, 0.f, 5.f) / 5.f; // 0–1 normalized
		float speed = SPEED_LOW + normSpeed * (SPEED_HIGH - SPEED_LOW);
	
		// --- Precalculate length with CV ---
		float knobLengthV = params[LENGTH_PARAM].getValue() * 5.f;  // 0–1 → 0–5V
		float lengthCVV = 0.f;
		if (inputs[LENGTHCVIN_INPUT].isConnected())
			lengthCVV = std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f; // -10..10 → -5..5
		float normLength = std::clamp(knobLengthV + lengthCVV, 0.f, 5.f) / 5.f; // 0–1 normalized
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
	
		// --- Process each voice ---
		processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, hatVoice, HATTRIGIN_INPUT, HATPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, rimVoice, RIMTRIGIN_INPUT, RIMPUSH_PARAM, speed, lengthRatio, loopEnabled);
	}
	

	void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId, float speed, float lengthRatio, bool loopEnabled) {
		const bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
		const bool buttonTrigger = params[pushParamId].getValue() > 0.5f;
	
		const bool inputRising = inputTrigger && !voice.lastInputTrigger;
		const bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;
	
		voice.lastInputTrigger = inputTrigger;
		voice.lastButtonTrigger = buttonTrigger;
	
		// --- New: fire flag for light control ---
		bool fired = false;
	
		// Manual trigger
		if (inputRising || buttonRising || (loopEnabled && !voice.playing)) {
			voice.playing = true;
			voice.samplePos = 0.f;
			fired = true;  // light should blink
			if (voice.lightId >= 0)
				lights[voice.lightId].setBrightness(1.f);
			voice.outputZeroSet = false;
		}
	
		if (voice.playing) {
			int maxSamplesToPlay = (int)(voice.decodedSample.size() * lengthRatio);
			int idx = (int)voice.samplePos;
	
			if (idx < maxSamplesToPlay) {
				float sample = voice.decodedSample[idx];
				outputs[voice.outputId].setVoltage(sample * 5.f);
				voice.samplePos += speed;
			} else {
				if (loopEnabled) {
					voice.samplePos = 0.f;
					fired = true; // 🔴 new: blink light on loop restart
				} else {
					voice.playing = false;
					outputs[voice.outputId].setVoltage(0.f);
					voice.outputZeroSet = true;
				}
			}
		} else {
			outputs[voice.outputId].setVoltage(0.f);
		}
	
		// Light handling
		if (voice.lightId >= 0) {
			if (fired)
				lights[voice.lightId].setBrightness(1.f);
			else
				lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
		}
	}	
};

struct DeeArrWidget : ModuleWidget {
	DeeArrWidget(DeeArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/DeeArr.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 19.001)), module, DeeArr::SPEED_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(9.751, 29.2)), module, DeeArr::LENGTH_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(41.25, 27.49)), module, DeeArr::LOOP_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(41.25, 27.49)), module, DeeArr::LOOP_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67.0)), module, DeeArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 67.0)), module, DeeArr::KICK_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82.0)), module, DeeArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 82.0)), module, DeeArr::SNARE_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97.0)), module, DeeArr::HATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 97.0)), module, DeeArr::HAT_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module, DeeArr::RIMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 112.0)), module, DeeArr::RIM_LIGHT));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 36.499)), module, DeeArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.751, 47.001)), module, DeeArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.25, 47.001)), module, DeeArr::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module, DeeArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module, DeeArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module, DeeArr::HATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module, DeeArr::RIMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module, DeeArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module, DeeArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module, DeeArr::HATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)), module, DeeArr::RIMOUT_OUTPUT));
	}
};


Model* modelDeeArr = createModel<DeeArr, DeeArrWidget>("DeeArr");