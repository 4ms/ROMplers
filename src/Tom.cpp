#include "plugin.hpp"
#include "TomSamples.hpp"

struct Tom : Module {
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
		TOM_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLengthSamples = 0;  // 16-bit samples count
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float TomLightBrightness = 0.f;

	constexpr static float MIN_PLAYBACK_SPEED = 0.01f;
	constexpr static float MAX_PLAYBACK_SPEED = 2.0f;
	constexpr static float SAMPLE_SAMPLE_RATE = 44100.f;

	int numSamples = 16;

	Tom() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		std::vector<std::string> sampleChoices;
		for (int i = 1; i <= numSamples; ++i)
			sampleChoices.push_back(std::to_string(i));
		configSwitch(SAMPLE_PARAM, 0.f, (numSamples-1), 0.f, "Sample", sampleChoices);
		configParam(PITCH_PARAM, -1.f, 1.f, 0.f, "Pitch", "%", 0.f, 100.f);
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");
		configInput(SAMPLECVIN_INPUT, "Sample CV");
		configInput(PITCHCVIN_INPUT, "Pitch CV");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(VOLCVIN_INPUT, "Volume CV");
		configInput(TRIGIN_INPUT, "Trig");
		configOutput(AUDIOOUT_OUTPUT, "Audio output");
	}

	const unsigned char* getSampleByIndex(int index) {
		switch (index) {
			case 0: return Tom1; case 1: return Tom2; case 2: return Tom3; case 3: return Tom4; case 4: return Tom5; case 5: return Tom6;
			case 6: return Tom7; case 7: return Tom8; case 8: return Tom9; case 9: return Tom10; case 10: return Tom11; case 11: return Tom12;
			case 12: return Tom13; case 13: return Tom14; case 14: return Tom15; case 15: return Tom16; 
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Tom1_len; case 1: return Tom2_len; case 2: return Tom3_len; case 3: return Tom4_len; case 4: return Tom5_len; case 5: return Tom6_len;
			case 6: return Tom7_len; case 7: return Tom8_len; case 8: return Tom9_len; case 9: return Tom10_len; case 10: return Tom11_len; case 11: return Tom12_len;
			case 12: return Tom13_len; case 13: return Tom14_len; case 14: return Tom15_len; case 15: return Tom16_len; 
			default: return 0;
		}
	}

	void process(const ProcessArgs& args) override {
		// Read inputs
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();
	
		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
	
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;
	
		bool triggered = trigRising || buttonRising;
	
		if (triggered) {
			TomLightBrightness = 1.0f;
	
			// --- Sample selection ---
			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue());
			if (inputs[SAMPLECVIN_INPUT].isConnected()) {
				float cv = std::clamp(inputs[SAMPLECVIN_INPUT].getVoltage(), -5.f, 5.f);
				int cvOffset = (int)round((cv / 5.f) * numSamples); // ±5V → ±numSamples
				sampleIndex += cvOffset;
			}
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);
	
			currentSample = getSampleByIndex(sampleIndex);
			int sampleLengthBytes = getSampleLengthByIndex(sampleIndex);
			sampleLengthSamples = sampleLengthBytes / 2; // 16-bit samples
	
			samplePos = 0.f;
			playing = (currentSample != nullptr && sampleLengthSamples > 1);
			env = 1.f;
		}
	
		// --- Light decay ---
		TomLightBrightness -= args.sampleTime * 10.f;
		if (TomLightBrightness < 0.f) TomLightBrightness = 0.f;
		lights[TOM_LIGHT].setBrightnessSmooth(TomLightBrightness, args.sampleTime);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			// --- Pitch CV ---
			float pitchMod = params[PITCH_PARAM].getValue();
			if (inputs[PITCHCVIN_INPUT].isConnected()) {
				float cv = std::clamp(inputs[PITCHCVIN_INPUT].getVoltage(), -5.f, 5.f);
				pitchMod += cv / 5.f; // ±5V → ±1
			}
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);
			float normalizedPitch = (pitchMod + 1.f) * 0.5f;
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
	
			float sampleRateRatio = SAMPLE_SAMPLE_RATE / args.sampleRate;
			samplePos += pitchRatio * sampleRateRatio;
	
			// --- Sample interpolation ---
			if ((int)samplePos >= sampleLengthSamples) {
				playing = false;
				env = 0.f;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < sampleLengthSamples) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				int16_t s1 = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2 = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));
	
				float sampleValue = ((s1 + frac * (s2 - s1)) / 32768.f);

				float knobV = params[DECAY_PARAM].getValue() * 5.f;
				float cvV = 0.f;
				if (inputs[DECAYCVIN_INPUT].isConnected())
    			cvV = std::clamp(inputs[DECAYCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f;
				float decayParam = std::clamp(knobV + cvV, 0.f, 5.f) / 5.f;
	
				const float minDecayTime = 0.005f;
				const float maxDecayTime = (float)sampleLengthSamples / SAMPLE_SAMPLE_RATE;
				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
				float decayCoef = expf(-1.f / (decayTime * args.sampleRate));
	
				env *= decayCoef;
				output = sampleValue * env;
			}
		} else {
			env = 0.f;
		}
	
		// --- Volume CV ---
		float volume = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volume = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
		}
		output *= volume / 5.f;
	
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 10.f);
	}	
};

struct TomWidget : ModuleWidget {
	TomWidget(Tom* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Tom.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, Tom::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, Tom::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, Tom::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, Tom::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, Tom::TOM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, Tom::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, Tom::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, Tom::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, Tom::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, Tom::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, Tom::AUDIOOUT_OUTPUT));
	}
};


Model* modelTom = createModel<Tom, TomWidget>("Tom");