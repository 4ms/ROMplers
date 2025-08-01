#include "plugin.hpp"
#include "RideSamples.hpp"

struct Ride : Module {
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
		RIDE_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float RideLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	int numSamples = 15;

	Ride() {
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
			case 0: return Ride1; case 1: return Ride2; case 2: return Ride3; case 3: return Ride4; case 4: return Ride5; case 5: return Ride6;
			case 6: return Ride7; case 7: return Ride8; case 8: return Ride9; case 9: return Ride10; case 10: return Ride11; case 11: return Ride12;
			case 12: return Ride13; case 13: return Ride14; case 14: return Ride15; 
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Ride1_len; case 1: return Ride2_len; case 2: return Ride3_len; case 3: return Ride4_len; case 4: return Ride5_len; case 5: return Ride6_len;
			case 6: return Ride7_len; case 7: return Ride8_len; case 8: return Ride9_len; case 9: return Ride10_len; case 10: return Ride11_len; case 11: return Ride12_len;
			case 12: return Ride13_len; case 13: return Ride14_len; case 14: return Ride15_len; 
			default: return 0;
		}
	}

	void process(const ProcessArgs& args) override {
		// Cache sample time and sample rate for efficiency
		const float sampleTime = args.sampleTime;
		const float sampleRate = args.sampleRate;
	
		// Trigger detection (rising edge on trig or button)
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();
	
		const bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		const bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
	
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;
	
		if (trigRising || buttonRising) {
			RideLightBrightness = 1.f;
	
			// Determine sample index clamped to valid range
			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = clamp(sampleIndex, 0, numSamples - 1);
	
			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;
	
			playing = (currentSample != nullptr && sampleLength > 1);
			env = 1.f;
		}
	
		// Light decay
		RideLightBrightness -= sampleTime * 10.f;
		if (RideLightBrightness < 0.f)
			RideLightBrightness = 0.f;
		lights[RIDE_LIGHT].setBrightnessSmooth(RideLightBrightness, sampleTime);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			// Pitch processing
			float pitchMod = params[PITCH_PARAM].getValue() + inputs[PITCHCVIN_INPUT].getVoltage();
			pitchMod = (pitchMod < -1.f) ? -1.f : (pitchMod > 1.f ? 1.f : pitchMod);
	
			// Convert pitch modulation to playback speed ratio
			float normalizedPitch = (pitchMod + 1.f) * 0.5f;
			const float MIN_PLAYBACK_SPEED = 0.01f;
			const float MAX_PLAYBACK_SPEED = 2.0f;
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
	
			constexpr float sampleSampleRate = 44100.f;
			float sampleRateRatio = sampleSampleRate / sampleRate;
	
			samplePos += pitchRatio * sampleRateRatio;
	
			const int numSamplesInSample = sampleLength / 2; // 16-bit samples, 2 bytes each
			if ((int)samplePos >= numSamplesInSample) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamplesInSample) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				// Read samples (little-endian)
				int16_t s1 = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2 = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));
	
				// Linear interpolation
				float sampleValue = s1 * (1.f - frac) + s2 * frac;
				sampleValue /= 32768.f;
	
				// Decay envelope
				float decayParam = params[DECAY_PARAM].getValue() + inputs[DECAYCVIN_INPUT].getVoltage();
				decayParam = (decayParam < 0.f) ? 0.f : (decayParam > 1.f ? 1.f : decayParam);
	
				// Precalculate decay time range
				constexpr float minDecayTime = 0.005f;
				float maxDecayTime = (float)numSamplesInSample / sampleSampleRate;
				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
	
				// Calculate decay coefficient once per sample frame
				float decayCoef = expf(-1.f / (decayTime * sampleRate));
	
				env *= decayCoef;
	
				output = sampleValue * env;
			}
		}
	
		// Volume CV processing (default to 1.0 if no input)
		float volumeCV = 1.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = inputs[VOLCVIN_INPUT].getVoltage();
			if (volumeCV < 0.f) volumeCV = 0.f;
			else if (volumeCV > 5.f) volumeCV = 5.f;
			volumeCV /= 5.f;
		}
	
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * volumeCV * 5.f);
	}	
};

struct RideWidget : ModuleWidget {
	RideWidget(Ride* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Ride_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 15.971)), module, Ride::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 39.997)), module, Ride::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 64.029)), module, Ride::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 89.342)), module, Ride::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 89.342)), module, Ride::RIDE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 29.578)), module, Ride::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 54.083)), module, Ride::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.658)), module, Ride::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 105.481)), module, Ride::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 105.481)), module, Ride::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119.24)), module, Ride::AUDIOOUT_OUTPUT));
	}
};


Model* modelRide = createModel<Ride, RideWidget>("Ride");