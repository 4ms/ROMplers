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
		float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		float buttonIn = params[PUSH_PARAM].getValue();

		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		bool triggered = trigRising || buttonRising;

		if (triggered) {
			RideLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = clamp(sampleIndex, 0, (numSamples-1));

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;

			playing = (currentSample != nullptr && sampleLength > 1);

			env = 1.0f;
		}

		RideLightBrightness = std::max(0.f, RideLightBrightness - (float)(args.sampleTime * 10.f));
		lights[RIDE_LIGHT].setBrightnessSmooth(RideLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSample) {
			float pitchMod = params[PITCH_PARAM].getValue() + inputs[PITCHCVIN_INPUT].getVoltage();
			pitchMod = clamp(pitchMod, -1.f, 1.f);

			float normalizedPitch = (pitchMod + 1.f) / 2.f;
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);

			constexpr float sampleSampleRate = 44100.f;
			float sampleRateRatio = sampleSampleRate / args.sampleRate;

			samplePos += pitchRatio * sampleRateRatio;

			int numSamples = sampleLength / 2;

			if ((int)samplePos >= numSamples) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamples) ? idx + 1 : idx;
				float frac = samplePos - idx;

				int16_t s1s = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2s = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));

				float s1 = (float)s1s / 32768.f;
				float s2 = (float)s2s / 32768.f;

				float sampleValue = s1 + frac * (s2 - s1);

				float decayParam = params[DECAY_PARAM].getValue() + inputs[DECAYCVIN_INPUT].getVoltage();
				decayParam = clamp(decayParam, 0.f, 1.f);

				float minDecayTime = 0.005f;
				float maxDecayTime = (float)numSamples / sampleSampleRate;

				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);

				float decayCoef = expf(-1.f / (decayTime * args.sampleRate));

				env = env * decayCoef;

				output = sampleValue * env;
			}
		}

float volumeCV = 5.f;
if (inputs[VOLCVIN_INPUT].isConnected()) {
	volumeCV = clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
}

output *= volumeCV / 5.f;

outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);	}
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