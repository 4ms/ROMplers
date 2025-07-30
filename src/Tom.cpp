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
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float kickLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	int numSamples = 80;

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
			case 12: return Tom13; case 13: return Tom14; case 14: return Tom15; case 15: return Tom16; case 16: return Tom17; case 17: return Tom18;
			case 18: return Tom19; case 19: return Tom20; case 20: return Tom21; case 21: return Tom22; case 22: return Tom23; case 23: return Tom24;
			case 24: return Tom25; case 25: return Tom26; case 26: return Tom27; case 27: return Tom28; case 28: return Tom29; case 29: return Tom30;
			case 30: return Tom31; case 31: return Tom32; case 32: return Tom33; case 33: return Tom34; case 34: return Tom35; case 35: return Tom36;
			case 36: return Tom37; case 37: return Tom38; case 38: return Tom39; case 39: return Tom40; case 40: return Tom41; case 41: return Tom42;
			case 42: return Tom43; case 43: return Tom44; case 44: return Tom45; case 45: return Tom46; case 46: return Tom47; case 47: return Tom48;
			case 48: return Tom49; case 49: return Tom50; case 50: return Tom51; case 51: return Tom52; case 52: return Tom53; case 53: return Tom54;
			case 54: return Tom55; case 55: return Tom56; case 56: return Tom57; case 57: return Tom58; case 58: return Tom59; case 59: return Tom60;
			case 60: return Tom61; case 61: return Tom62; case 62: return Tom63; case 63: return Tom64; case 64: return Tom65; case 65: return Tom66;
			case 66: return Tom67; case 67: return Tom68; case 68: return Tom69; case 69: return Tom70; case 70: return Tom71; case 71: return Tom72;
			case 72: return Tom73; case 73: return Tom74; case 74: return Tom75; case 75: return Tom76; case 76: return Tom77; case 77: return Tom78;
			case 78: return Tom79; case 79: return Tom80;
			default: return nullptr;
		}
	}
	

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Tom1_len; case 1: return Tom2_len; case 2: return Tom3_len; case 3: return Tom4_len; case 4: return Tom5_len; case 5: return Tom6_len;
			case 6: return Tom7_len; case 7: return Tom8_len; case 8: return Tom9_len; case 9: return Tom10_len; case 10: return Tom11_len; case 11: return Tom12_len;
			case 12: return Tom13_len; case 13: return Tom14_len; case 14: return Tom15_len; case 15: return Tom16_len; case 16: return Tom17_len; case 17: return Tom18_len;
			case 18: return Tom19_len; case 19: return Tom20_len; case 20: return Tom21_len; case 21: return Tom22_len; case 22: return Tom23_len; case 23: return Tom24_len;
			case 24: return Tom25_len; case 25: return Tom26_len; case 26: return Tom27_len; case 27: return Tom28_len; case 28: return Tom29_len; case 29: return Tom30_len;
			case 30: return Tom31_len; case 31: return Tom32_len; case 32: return Tom33_len; case 33: return Tom34_len; case 34: return Tom35_len; case 35: return Tom36_len;
			case 36: return Tom37_len; case 37: return Tom38_len; case 38: return Tom39_len; case 39: return Tom40_len; case 40: return Tom41_len; case 41: return Tom42_len;
			case 42: return Tom43_len; case 43: return Tom44_len; case 44: return Tom45_len; case 45: return Tom46_len; case 46: return Tom47_len; case 47: return Tom48_len;
			case 48: return Tom49_len; case 49: return Tom50_len; case 50: return Tom51_len; case 51: return Tom52_len; case 52: return Tom53_len; case 53: return Tom54_len;
			case 54: return Tom55_len; case 55: return Tom56_len; case 56: return Tom57_len; case 57: return Tom58_len; case 58: return Tom59_len; case 59: return Tom60_len;
			case 60: return Tom61_len; case 61: return Tom62_len; case 62: return Tom63_len; case 63: return Tom64_len; case 64: return Tom65_len; case 65: return Tom66_len;
			case 66: return Tom67_len; case 67: return Tom68_len; case 68: return Tom69_len; case 69: return Tom70_len; case 70: return Tom71_len; case 71: return Tom72_len;
			case 72: return Tom73_len; case 73: return Tom74_len; case 74: return Tom75_len; case 75: return Tom76_len; case 76: return Tom77_len; case 77: return Tom78_len;
			case 78: return Tom79_len; case 79: return Tom80_len; 
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
			kickLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = clamp(sampleIndex, 0, (numSamples-1));

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;

			playing = (currentSample != nullptr && sampleLength > 1);

			env = 1.0f;
		}

		kickLightBrightness = std::max(0.f, kickLightBrightness - (float)(args.sampleTime * 10.f));
		lights[TOM_LIGHT].setBrightnessSmooth(kickLightBrightness, args.sampleTime);

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

struct TomWidget : ModuleWidget {
	TomWidget(Tom* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Tom_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 15.971)), module, Tom::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 39.997)), module, Tom::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 64.029)), module, Tom::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 89.342)), module, Tom::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 89.342)), module, Tom::TOM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 29.578)), module, Tom::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 54.083)), module, Tom::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.658)), module, Tom::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 105.481)), module, Tom::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 105.481)), module, Tom::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119.24)), module, Tom::AUDIOOUT_OUTPUT));
	}
};


Model* modelTom = createModel<Tom, TomWidget>("Tom");