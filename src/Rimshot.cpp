#include "plugin.hpp"
#include "RimshotSamples.hpp"

struct Rimshot : Module {
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
		RIMSHOT_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float RimshotLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	constexpr static float sampleSampleRate = 44100.f; // constant

	int numSamples = 9;

	Rimshot() {
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
			case 0: return Rimshot1; case 1: return Rimshot2; case 2: return Rimshot3; case 3: return Rimshot4; case 4: return Rimshot5;
			case 5: return Rimshot6; case 6: return Rimshot7; case 7: return Rimshot8; case 8: return Rimshot9;
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Rimshot1_len; case 1: return Rimshot2_len; case 2: return Rimshot3_len; case 3: return Rimshot4_len; case 4: return Rimshot5_len;
			case 5: return Rimshot6_len; case 6: return Rimshot7_len; case 7: return Rimshot8_len; case 8: return Rimshot9_len;
			default: return 0;
		}
	}

	void process(const ProcessArgs& args) override {
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();

		const bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		const bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		if (trigRising || buttonRising) {
			RimshotLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;

			playing = (currentSample != nullptr && sampleLength > 1);
			env = 1.0f;
		}

		// Decay the light brightness smoothly, cheaper to do once per frame
		if (RimshotLightBrightness > 0.f) {
			RimshotLightBrightness -= args.sampleTime * 10.f;
			if (RimshotLightBrightness < 0.f)
				RimshotLightBrightness = 0.f;
		}
		lights[RIMSHOT_LIGHT].setBrightnessSmooth(RimshotLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSample) {
			// Cache parameters to avoid repeated calls & std::clamp once
			float pitchMod = std::clamp(params[PITCH_PARAM].getValue() + inputs[PITCHCVIN_INPUT].getVoltage(), -1.f, 1.f);
			const float normalizedPitch = 0.5f * (pitchMod + 1.f);
			const float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
			const float sampleRateRatio = sampleSampleRate / args.sampleRate;

			samplePos += pitchRatio * sampleRateRatio;

			const int numSamples = sampleLength / 2;

			const int idx = (int)samplePos;
			if (idx >= numSamples) {
				playing = false;
				output = 0.f;
			} else {
				const int nextIdx = (idx + 1 < numSamples) ? idx + 1 : idx;
				const float frac = samplePos - idx;

				// Load samples as int16_t
				const int16_t s1 = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				const int16_t s2 = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));

				const float sampleValue = (float)s1 + frac * ((float)s2 - (float)s1);
				const float normalizedSample = sampleValue * (1.f / 32768.f);

				float decayParam = std::clamp(params[DECAY_PARAM].getValue() + inputs[DECAYCVIN_INPUT].getVoltage(), 0.f, 1.f);
				const float minDecayTime = 0.005f;
				const float maxDecayTime = (float)numSamples / sampleSampleRate;
				const float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
				const float decayCoef = expf(-1.f / (decayTime * args.sampleRate));

				env *= decayCoef;

				output = normalizedSample * env;
			}
		}

		float volumeCV = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected())
			volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);

		output *= (volumeCV / 5.f);
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);
	}
};

struct RimshotWidget : ModuleWidget {
	RimshotWidget(Rimshot* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Rimshot_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 15.971)), module, Rimshot::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 39.997)), module, Rimshot::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 64.029)), module, Rimshot::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 89.342)), module, Rimshot::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 89.342)), module, Rimshot::RIMSHOT_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 29.578)), module, Rimshot::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 54.083)), module, Rimshot::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.658)), module, Rimshot::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 105.481)), module, Rimshot::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 105.481)), module, Rimshot::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119.24)), module, Rimshot::AUDIOOUT_OUTPUT));
	}
};


Model* modelRimshot = createModel<Rimshot, RimshotWidget>("Rimshot");