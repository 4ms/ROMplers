#include "plugin.hpp"
#include "ClosedHatSamples.hpp"

struct ClosedHat : Module {
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
		CLOSEDHAT_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float ClosedHatLightBrightness = 0.f;

	// Precomputed per-trigger:
	float decayCoef = 0.f;
	float pitchRatio = 1.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	int numSamples = 21;

	ClosedHat() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		std::vector<std::string> sampleChoices;
		for (int i = 1; i <= numSamples; ++i)
			sampleChoices.push_back(std::to_string(i));
		configSwitch(SAMPLE_PARAM, 0.f, (numSamples - 1), 0.f, "Sample", sampleChoices);
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
			case 0: return ClosedHat1; case 1: return ClosedHat2; case 2: return ClosedHat3; case 3: return ClosedHat4; case 4: return ClosedHat5; case 5: return ClosedHat6;
			case 6: return ClosedHat7; case 7: return ClosedHat8; case 8: return ClosedHat9; case 9: return ClosedHat10; case 10: return ClosedHat11; case 11: return ClosedHat12;
			case 12: return ClosedHat13; case 13: return ClosedHat14; case 14: return ClosedHat15; case 15: return ClosedHat16; case 16: return ClosedHat17; case 17: return ClosedHat18;
			case 18: return ClosedHat19; case 19: return ClosedHat20; case 20: return ClosedHat21;
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return ClosedHat1_len; case 1: return ClosedHat2_len; case 2: return ClosedHat3_len; case 3: return ClosedHat4_len; case 4: return ClosedHat5_len; case 5: return ClosedHat6_len;
			case 6: return ClosedHat7_len; case 7: return ClosedHat8_len; case 8: return ClosedHat9_len; case 9: return ClosedHat10_len; case 10: return ClosedHat11_len; case 11: return ClosedHat12_len;
			case 12: return ClosedHat13_len; case 13: return ClosedHat14_len; case 14: return ClosedHat15_len; case 15: return ClosedHat16_len; case 16: return ClosedHat17_len; case 17: return ClosedHat18_len;
			case 18: return ClosedHat19_len; case 19: return ClosedHat20_len; case 20: return ClosedHat21_len;
			default: return 0;
		}
	}

	void process(const ProcessArgs& args) override {
		// Cache inputs and params once
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();
		const float sampleCV = inputs[SAMPLECVIN_INPUT].getVoltage();
		const float pitchCV = inputs[PITCHCVIN_INPUT].getVoltage();
		const float decayCV = inputs[DECAYCVIN_INPUT].getVoltage();
		const float volCV = inputs[VOLCVIN_INPUT].isConnected() ? std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f) : 5.f;

		// Trigger detection
		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		const bool triggered = trigRising || buttonRising;

		if (triggered) {
			ClosedHatLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + sampleCV);
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;
			playing = (currentSample != nullptr && sampleLength > 1);

			env = 1.0f;

			// Precompute decay coefficient
			float decayParam = std::clamp(params[DECAY_PARAM].getValue() + decayCV, 0.f, 1.f);
			const float minDecayTime = 0.005f;
			const float maxDecayTime = (float)(sampleLength / 2) / 44100.f;
			float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
			decayCoef = expf(-1.f / (decayTime * args.sampleRate));

			// Precompute pitch ratio
			float pitchMod = std::clamp(params[PITCH_PARAM].getValue() + pitchCV, -1.f, 1.f);
			float normalizedPitch = (pitchMod + 1.f) / 2.f;
			pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
			pitchRatio *= 44100.f / args.sampleRate;
		}

		// Light decay
		ClosedHatLightBrightness = std::max(0.f, ClosedHatLightBrightness - (float)(args.sampleTime * 10.f));
		lights[CLOSEDHAT_LIGHT].setBrightnessSmooth(ClosedHatLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSample) {
			const int numSamples = sampleLength / 2;
			int idx = (int)samplePos;
			if (idx >= numSamples) {
				playing = false;
			} else {
				int nextIdx = (idx + 1 < numSamples) ? idx + 1 : idx;
				const float frac = samplePos - idx;

				const uint8_t* samplePtr = currentSample;
				const int16_t s1s = (int16_t)(samplePtr[idx * 2] | (samplePtr[idx * 2 + 1] << 8));
				const int16_t s2s = (int16_t)(samplePtr[nextIdx * 2] | (samplePtr[nextIdx * 2 + 1] << 8));

				const float s1 = s1s * (1.0f / 32768.f);
				const float s2 = s2s * (1.0f / 32768.f);
				const float sampleValue = s1 + frac * (s2 - s1);

				env *= decayCoef;

				output = sampleValue * env;

				samplePos += pitchRatio;
			}
		}

		output *= volCV / 5.f;
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.f);
	}
};

struct ClosedHatWidget : ModuleWidget {
	ClosedHatWidget(ClosedHat* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/ClosedHat.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, ClosedHat::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, ClosedHat::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, ClosedHat::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, ClosedHat::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, ClosedHat::CLOSEDHAT_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, ClosedHat::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, ClosedHat::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, ClosedHat::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, ClosedHat::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, ClosedHat::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, ClosedHat::AUDIOOUT_OUTPUT));
	}
};


Model* modelClosedHat = createModel<ClosedHat, ClosedHatWidget>("ClosedHat");