#include "plugin.hpp"
#include "ClapSamples.hpp"

struct Clap : Module {
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
		Clap_LIGHT,
		LIGHTS_LEN
	};

	// Pre-decoded sample buffer (float samples)
	std::vector<float> decodedSample;
	const float* currentSampleFloat = nullptr;
	int decodedSampleLength = 0;

	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float ClapLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	int numSamples = 9;

	Clap() {
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
			case 0: return Clap1;
			case 1: return Clap2;
			case 2: return Clap3;
			case 3: return Clap4;
			case 4: return Clap5;
			case 5: return Clap6;
			case 6: return Clap7;
			case 7: return Clap8;
			case 8: return Clap9;
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Clap1_len;
			case 1: return Clap2_len;
			case 2: return Clap3_len;
			case 3: return Clap4_len;
			case 4: return Clap5_len;
			case 5: return Clap6_len;
			case 6: return Clap7_len;
			case 7: return Clap8_len;
			case 8: return Clap9_len;
			default: return 0;
		}
	}

	// Predecode sample bytes to float buffer once per sample change
	void loadSample(int index) {
		const unsigned char* sampleData = getSampleByIndex(index);
		int length = getSampleLengthByIndex(index);

		if (!sampleData || length < 2) {
			currentSampleFloat = nullptr;
			decodedSampleLength = 0;
			decodedSample.clear();
			return;
		}

		int numSamplesInt = length / 2;
		decodedSample.resize(numSamplesInt);

		for (int i = 0; i < numSamplesInt; ++i) {
			int16_t s = (int16_t)(sampleData[i * 2] | (sampleData[i * 2 + 1] << 8));
			decodedSample[i] = (float)s / 32768.f;
		}

		currentSampleFloat = decodedSample.data();
		decodedSampleLength = numSamplesInt;
	}

	void process(const ProcessArgs& args) override {
		float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		float buttonIn = params[PUSH_PARAM].getValue();

		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		bool triggered = trigRising || buttonRising;

		if (triggered) {
			ClapLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = clamp(sampleIndex, 0, numSamples - 1);

			loadSample(sampleIndex);

			samplePos = 0.f;
			playing = (currentSampleFloat != nullptr && decodedSampleLength > 1);

			env = 1.0f;
		}

		ClapLightBrightness = std::max(0.f, ClapLightBrightness - (float)(args.sampleTime * 10.f));
		lights[Clap_LIGHT].setBrightnessSmooth(ClapLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSampleFloat) {
			// Cache parameters once
			float pitchMod = clamp(params[PITCH_PARAM].getValue() + inputs[PITCHCVIN_INPUT].getVoltage(), -1.f, 1.f);
			float normalizedPitch = (pitchMod + 1.f) * 0.5f;
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);

			constexpr float sampleSampleRate = 44100.f;
			float sampleRateRatio = sampleSampleRate / args.sampleRate;

			float decayParam = clamp(params[DECAY_PARAM].getValue() + inputs[DECAYCVIN_INPUT].getVoltage(), 0.f, 1.f);
			float minDecayTime = 0.005f;
			float maxDecayTime = (float)decodedSampleLength / sampleSampleRate;
			float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
			float decayCoef = expf(-1.f / (decayTime * args.sampleRate));

			// Advance sample position
			samplePos += pitchRatio * sampleRateRatio;

			if ((int)samplePos >= decodedSampleLength) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < decodedSampleLength) ? idx + 1 : idx;
				float frac = samplePos - idx;

				// Linear interpolate between floats (predecoded)
				float sampleValue = currentSampleFloat[idx] + frac * (currentSampleFloat[nextIdx] - currentSampleFloat[idx]);

				env *= decayCoef;

				output = sampleValue * env;
			}
		}

		float volumeCV = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
		}

		output *= volumeCV / 5.f;

		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);
	}
};

struct ClapWidget : ModuleWidget {
	ClapWidget(Clap* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Clap_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 15.971)), module, Clap::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 39.997)), module, Clap::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 64.029)), module, Clap::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 89.342)), module, Clap::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 89.342)), module, Clap::Clap_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 29.578)), module, Clap::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 54.083)), module, Clap::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.658)), module, Clap::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 105.481)), module, Clap::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 105.481)), module, Clap::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119.24)), module, Clap::AUDIOOUT_OUTPUT));
	}
};


Model* modelClap = createModel<Clap, ClapWidget>("Clap");