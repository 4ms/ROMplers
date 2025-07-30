#include "plugin.hpp"
#include "SnareSamples.hpp"

struct Snare : Module {
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
		SNARE_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float SnareLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	int numSamples = 79;

	Snare() {
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
			case 0: return Snare1; case 1: return Snare2; case 2: return Snare3; case 3: return Snare4; case 4: return Snare5; case 5: return Snare6;
			case 6: return Snare7; case 7: return Snare8; case 8: return Snare9; case 9: return Snare10; case 10: return Snare11; case 11: return Snare12;
			case 12: return Snare13; case 13: return Snare14; case 14: return Snare15; case 15: return Snare16; case 16: return Snare17; case 17: return Snare18;
			case 18: return Snare19; case 19: return Snare20; case 20: return Snare21; case 21: return Snare22; case 22: return Snare23; case 23: return Snare24;
			case 24: return Snare25; case 25: return Snare26; case 26: return Snare27; case 27: return Snare28; case 28: return Snare29; case 29: return Snare30;
			case 30: return Snare31; case 31: return Snare32; case 32: return Snare33; case 33: return Snare34; case 34: return Snare35; case 35: return Snare36;
			case 36: return Snare37; case 37: return Snare38; case 38: return Snare39; case 39: return Snare40; case 40: return Snare41; case 41: return Snare42;
			case 42: return Snare43; case 43: return Snare44; case 44: return Snare45; case 45: return Snare46; case 46: return Snare47; case 47: return Snare48;
			case 48: return Snare49; case 49: return Snare50; case 50: return Snare51; case 51: return Snare52; case 52: return Snare53; case 53: return Snare54;
			case 54: return Snare55; case 55: return Snare56; case 56: return Snare57; case 57: return Snare58; case 58: return Snare59; case 59: return Snare60;
			case 60: return Snare61; case 61: return Snare62; case 62: return Snare63; case 63: return Snare64; case 64: return Snare65; case 65: return Snare66;
			case 66: return Snare67; case 67: return Snare68; case 68: return Snare69; case 69: return Snare70; case 70: return Snare71; case 71: return Snare72;
			case 72: return Snare73; case 73: return Snare74; case 74: return Snare75; case 75: return Snare76; case 76: return Snare77; case 77: return Snare78;
			case 78: return Snare79;
			default: return nullptr;
		}
	}
	

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Snare1_len; case 1: return Snare2_len; case 2: return Snare3_len; case 3: return Snare4_len; case 4: return Snare5_len; case 5: return Snare6_len;
			case 6: return Snare7_len; case 7: return Snare8_len; case 8: return Snare9_len; case 9: return Snare10_len; case 10: return Snare11_len; case 11: return Snare12_len;
			case 12: return Snare13_len; case 13: return Snare14_len; case 14: return Snare15_len; case 15: return Snare16_len; case 16: return Snare17_len; case 17: return Snare18_len;
			case 18: return Snare19_len; case 19: return Snare20_len; case 20: return Snare21_len; case 21: return Snare22_len; case 22: return Snare23_len; case 23: return Snare24_len;
			case 24: return Snare25_len; case 25: return Snare26_len; case 26: return Snare27_len; case 27: return Snare28_len; case 28: return Snare29_len; case 29: return Snare30_len;
			case 30: return Snare31_len; case 31: return Snare32_len; case 32: return Snare33_len; case 33: return Snare34_len; case 34: return Snare35_len; case 35: return Snare36_len;
			case 36: return Snare37_len; case 37: return Snare38_len; case 38: return Snare39_len; case 39: return Snare40_len; case 40: return Snare41_len; case 41: return Snare42_len;
			case 42: return Snare43_len; case 43: return Snare44_len; case 44: return Snare45_len; case 45: return Snare46_len; case 46: return Snare47_len; case 47: return Snare48_len;
			case 48: return Snare49_len; case 49: return Snare50_len; case 50: return Snare51_len; case 51: return Snare52_len; case 52: return Snare53_len; case 53: return Snare54_len;
			case 54: return Snare55_len; case 55: return Snare56_len; case 56: return Snare57_len; case 57: return Snare58_len; case 58: return Snare59_len; case 59: return Snare60_len;
			case 60: return Snare61_len; case 61: return Snare62_len; case 62: return Snare63_len; case 63: return Snare64_len; case 64: return Snare65_len; case 65: return Snare66_len;
			case 66: return Snare67_len; case 67: return Snare68_len; case 68: return Snare69_len; case 69: return Snare70_len; case 70: return Snare71_len; case 71: return Snare72_len;
			case 72: return Snare73_len; case 73: return Snare74_len; case 74: return Snare75_len; case 75: return Snare76_len; case 76: return Snare77_len; case 77: return Snare78_len;
			case 78: return Snare79_len;
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
			SnareLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = clamp(sampleIndex, 0, (numSamples-1));

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;

			playing = (currentSample != nullptr && sampleLength > 1);

			env = 1.0f;
		}

		SnareLightBrightness = std::max(0.f, SnareLightBrightness - (float)(args.sampleTime * 10.f));
		lights[SNARE_LIGHT].setBrightnessSmooth(SnareLightBrightness, args.sampleTime);

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

struct SnareWidget : ModuleWidget {
	SnareWidget(Snare* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Snare_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 15.971)), module, Snare::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 39.997)), module, Snare::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 64.029)), module, Snare::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 89.342)), module, Snare::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 89.342)), module, Snare::SNARE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 29.578)), module, Snare::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 54.083)), module, Snare::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.658)), module, Snare::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 105.481)), module, Snare::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 105.481)), module, Snare::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119.24)), module, Snare::AUDIOOUT_OUTPUT));
	}
};


Model* modelSnare = createModel<Snare, SnareWidget>("Snare");