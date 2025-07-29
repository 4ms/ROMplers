#include "plugin.hpp"
#include "KickSamples.hpp"

struct Kick : Module {
	enum ParamId {
		SAMPLE_PARAM,
		PITCH_PARAM,
		DECAY_PARAM,
		KICKPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SAMPLECVIN_INPUT,
		PITCHCVIN_INPUT,
		DECAYCVIN_INPUT,
		TRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float kickLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	Kick() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		std::vector<std::string> sampleChoices;
		for (int i = 1; i <= 60; ++i)
			sampleChoices.push_back(std::to_string(i));
		configSwitch(SAMPLE_PARAM, 0.f, 59.f, 0.f, "Sample", sampleChoices);
		configParam(PITCH_PARAM, -1.f, 1.f, 0.f, "Pitch", "%", 0.f, 100.f);
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
		configParam(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");
		configInput(SAMPLECVIN_INPUT, "Sample CV");
		configInput(PITCHCVIN_INPUT, "Pitch CV");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(TRIGIN_INPUT, "Trig");
		configOutput(AUDIOOUT_OUTPUT, "Audio output");
	}

	const unsigned char* getSampleByIndex(int index) {
		switch (index) {
			case 0: return Kick1; case 1: return Kick2; case 2: return Kick3; case 3: return Kick4; case 4: return Kick5; case 5: return Kick6;
			case 6: return Kick7; case 7: return Kick8; case 8: return Kick9; case 9: return Kick10; case 10: return Kick11; case 11: return Kick12;
			case 12: return Kick13; case 13: return Kick14; case 14: return Kick15; case 15: return Kick16; case 16: return Kick17; case 17: return Kick18;
			case 18: return Kick19; case 19: return Kick20; case 20: return Kick21; case 21: return Kick22; case 22: return Kick23; case 23: return Kick24;
			case 24: return Kick25; case 25: return Kick26; case 26: return Kick27; case 27: return Kick28; case 28: return Kick29; case 29: return Kick30;
			case 30: return Kick31; case 31: return Kick32; case 32: return Kick33; case 33: return Kick34; case 34: return Kick35; case 35: return Kick36;
			case 36: return Kick37; case 37: return Kick38; case 38: return Kick39; case 39: return Kick40; case 40: return Kick41; case 41: return Kick42;
			case 42: return Kick43; case 43: return Kick44; case 44: return Kick45; case 45: return Kick46; case 46: return Kick47; case 47: return Kick48;
			case 48: return Kick49; case 49: return Kick50; case 50: return Kick51; case 51: return Kick52; case 52: return Kick53; case 53: return Kick54;
			case 54: return Kick55; case 55: return Kick56; case 56: return Kick57; case 57: return Kick58; case 58: return Kick59; case 59: return Kick60;
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Kick1_len; case 1: return Kick2_len; case 2: return Kick3_len; case 3: return Kick4_len; case 4: return Kick5_len; case 5: return Kick6_len;
			case 6: return Kick7_len; case 7: return Kick8_len; case 8: return Kick9_len; case 9: return Kick10_len; case 10: return Kick11_len; case 11: return Kick12_len;
			case 12: return Kick13_len; case 13: return Kick14_len; case 14: return Kick15_len; case 15: return Kick16_len; case 16: return Kick17_len; case 17: return Kick18_len;
			case 18: return Kick19_len; case 19: return Kick20_len; case 20: return Kick21_len; case 21: return Kick22_len; case 22: return Kick23_len; case 23: return Kick24_len;
			case 24: return Kick25_len; case 25: return Kick26_len; case 26: return Kick27_len; case 27: return Kick28_len; case 28: return Kick29_len; case 29: return Kick30_len;
			case 30: return Kick31_len; case 31: return Kick32_len; case 32: return Kick33_len; case 33: return Kick34_len; case 34: return Kick35_len; case 35: return Kick36_len;
			case 36: return Kick37_len; case 37: return Kick38_len; case 38: return Kick39_len; case 39: return Kick40_len; case 40: return Kick41_len; case 41: return Kick42_len;
			case 42: return Kick43_len; case 43: return Kick44_len; case 44: return Kick45_len; case 45: return Kick46_len; case 46: return Kick47_len; case 47: return Kick48_len;
			case 48: return Kick49_len; case 49: return Kick50_len; case 50: return Kick51_len; case 51: return Kick52_len; case 52: return Kick53_len; case 53: return Kick54_len;
			case 54: return Kick55_len; case 55: return Kick56_len; case 56: return Kick57_len; case 57: return Kick58_len; case 58: return Kick59_len; case 59: return Kick60_len;
			default: return 0;
		}
	}

	void process(const ProcessArgs& args) override {
		float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		float buttonIn = params[KICKPUSH_PARAM].getValue();

		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		bool triggered = trigRising || buttonRising;

		if (triggered) {
			kickLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = clamp(sampleIndex, 0, 59);

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;

			playing = (currentSample != nullptr && sampleLength > 1);

			env = 1.0f;
		}

		kickLightBrightness = std::max(0.f, kickLightBrightness - (float)(args.sampleTime * 10.f));
		lights[KICK_LIGHT].setBrightnessSmooth(kickLightBrightness, args.sampleTime);

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

		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);
	}
};

struct KickWidget : ModuleWidget {
	KickWidget(Kick* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Kick_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 21.792)), module, Kick::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 45.818)), module, Kick::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 69.85)), module, Kick::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 95.162)), module, Kick::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 95.162)), module, Kick::KICK_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 35.399)), module, Kick::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 59.903)), module, Kick::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 85.479)), module, Kick::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 113.419)), module, Kick::TRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.317, 113.419)), module, Kick::AUDIOOUT_OUTPUT));
	}
};


Model* modelKick = createModel<Kick, KickWidget>("Kick");