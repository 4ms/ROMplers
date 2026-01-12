#include "plugin.hpp"
#include "PercussionSamples.hpp"

struct Percussion : Module {
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
		PERCUSSION_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float PercussionLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	int numSamples = 96;

	Percussion() {
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
			case 0: return Percussion1; case 1: return Percussion2; case 2: return Percussion3; case 3: return Percussion4;
			case 4: return Percussion5; case 5: return Percussion6; case 6: return Percussion7; case 7: return Percussion8;
			case 8: return Percussion9; case 9: return Percussion10; case 10: return Percussion11; case 11: return Percussion12;
			case 12: return Percussion13; case 13: return Percussion14; case 14: return Percussion15; case 15: return Percussion16;
			case 16: return Percussion17; case 17: return Percussion18; case 18: return Percussion19; case 19: return Percussion20;
			case 20: return Percussion21; case 21: return Percussion22; case 22: return Percussion23; case 23: return Percussion24;
			case 24: return Percussion25; case 25: return Percussion26; case 26: return Percussion27; case 27: return Percussion28;
			case 28: return Percussion29; case 29: return Percussion30; case 30: return Percussion31; case 31: return Percussion32;
			case 32: return Percussion33; case 33: return Percussion34; case 34: return Percussion35; case 35: return Percussion36;
			case 36: return Percussion37; case 37: return Percussion38; case 38: return Percussion39; case 39: return Percussion40;
			case 40: return Percussion41; case 41: return Percussion42; case 42: return Percussion43; case 43: return Percussion44;
			case 44: return Percussion45; case 45: return Percussion46; case 46: return Percussion47; case 47: return Percussion48;
			case 48: return Percussion49; case 49: return Percussion50; case 50: return Percussion51; case 51: return Percussion52;
			case 52: return Percussion53; case 53: return Percussion54; case 54: return Percussion55; case 55: return Percussion56;
			case 56: return Percussion57; case 57: return Percussion58; case 58: return Percussion59; case 59: return Percussion60;
			case 60: return Percussion61; case 61: return Percussion62; case 62: return Percussion63; case 63: return Percussion64;
			case 64: return Percussion65; case 65: return Percussion66; case 66: return Percussion67; case 67: return Percussion68;
			case 68: return Percussion69; case 69: return Percussion70; case 70: return Percussion71; case 71: return Percussion72;
			case 72: return Percussion73; case 73: return Percussion74; case 74: return Percussion75; case 75: return Percussion76;
			case 76: return Percussion77; case 77: return Percussion78; case 78: return Percussion79; case 79: return Percussion80;
			case 80: return Percussion81; case 81: return Percussion82; case 82: return Percussion83; case 83: return Percussion84;
			case 84: return Percussion85; case 85: return Percussion86; case 86: return Percussion87; case 87: return Percussion88;
			case 88: return Percussion89; case 89: return Percussion90; case 90: return Percussion91; case 91: return Percussion92;
			case 92: return Percussion93; case 93: return Percussion94; case 94: return Percussion95; case 95: return Percussion96;
			default: return nullptr;
		}
	}	

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Percussion1_len; case 1: return Percussion2_len; case 2: return Percussion3_len; case 3: return Percussion4_len;
			case 4: return Percussion5_len; case 5: return Percussion6_len; case 6: return Percussion7_len; case 7: return Percussion8_len;
			case 8: return Percussion9_len; case 9: return Percussion10_len; case 10: return Percussion11_len; case 11: return Percussion12_len;
			case 12: return Percussion13_len; case 13: return Percussion14_len; case 14: return Percussion15_len; case 15: return Percussion16_len;
			case 16: return Percussion17_len; case 17: return Percussion18_len; case 18: return Percussion19_len; case 19: return Percussion20_len;
			case 20: return Percussion21_len; case 21: return Percussion22_len; case 22: return Percussion23_len; case 23: return Percussion24_len;
			case 24: return Percussion25_len; case 25: return Percussion26_len; case 26: return Percussion27_len; case 27: return Percussion28_len;
			case 28: return Percussion29_len; case 29: return Percussion30_len; case 30: return Percussion31_len; case 31: return Percussion32_len;
			case 32: return Percussion33_len; case 33: return Percussion34_len; case 34: return Percussion35_len; case 35: return Percussion36_len;
			case 36: return Percussion37_len; case 37: return Percussion38_len; case 38: return Percussion39_len; case 39: return Percussion40_len;
			case 40: return Percussion41_len; case 41: return Percussion42_len; case 42: return Percussion43_len; case 43: return Percussion44_len;
			case 44: return Percussion45_len; case 45: return Percussion46_len; case 46: return Percussion47_len; case 47: return Percussion48_len;
			case 48: return Percussion49_len; case 49: return Percussion50_len; case 50: return Percussion51_len; case 51: return Percussion52_len;
			case 52: return Percussion53_len; case 53: return Percussion54_len; case 54: return Percussion55_len; case 55: return Percussion56_len;
			case 56: return Percussion57_len; case 57: return Percussion58_len; case 58: return Percussion59_len; case 59: return Percussion60_len;
			case 60: return Percussion61_len; case 61: return Percussion62_len; case 62: return Percussion63_len; case 63: return Percussion64_len;
			case 64: return Percussion65_len; case 65: return Percussion66_len; case 66: return Percussion67_len; case 67: return Percussion68_len;
			case 68: return Percussion69_len; case 69: return Percussion70_len; case 70: return Percussion71_len; case 71: return Percussion72_len;
			case 72: return Percussion73_len; case 73: return Percussion74_len; case 74: return Percussion75_len; case 75: return Percussion76_len;
			case 76: return Percussion77_len; case 77: return Percussion78_len; case 78: return Percussion79_len; case 79: return Percussion80_len;
			case 80: return Percussion81_len; case 81: return Percussion82_len; case 82: return Percussion83_len; case 83: return Percussion84_len;
			case 84: return Percussion85_len; case 85: return Percussion86_len; case 86: return Percussion87_len; case 87: return Percussion88_len;
			case 88: return Percussion89_len; case 89: return Percussion90_len; case 90: return Percussion91_len; case 91: return Percussion92_len;
			case 92: return Percussion93_len; case 93: return Percussion94_len; case 94: return Percussion95_len; case 95: return Percussion96_len;
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
			PercussionLightBrightness = 1.0f;

			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue() + inputs[SAMPLECVIN_INPUT].getVoltage());
			sampleIndex = std::clamp(sampleIndex, 0, (numSamples-1));

			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;

			playing = (currentSample != nullptr && sampleLength > 1);

			env = 1.0f;
		}

		PercussionLightBrightness = std::max(0.f, PercussionLightBrightness - (float)(args.sampleTime * 10.f));
		lights[PERCUSSION_LIGHT].setBrightnessSmooth(PercussionLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSample) {
			float pitchMod = params[PITCH_PARAM].getValue() + inputs[PITCHCVIN_INPUT].getVoltage();
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);

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
				decayParam = std::clamp(decayParam, 0.f, 1.f);

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
	volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
}

output *= volumeCV / 5.f;

outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);	}
};

struct PercussionWidget : ModuleWidget {
	PercussionWidget(Percussion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Percussion_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 15.971)), module, Percussion::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 39.997)), module, Percussion::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 64.029)), module, Percussion::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 89.342)), module, Percussion::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 89.342)), module, Percussion::PERCUSSION_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 29.578)), module, Percussion::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 54.083)), module, Percussion::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.658)), module, Percussion::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 105.481)), module, Percussion::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 105.481)), module, Percussion::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119.24)), module, Percussion::AUDIOOUT_OUTPUT));
	}
};


Model* modelPercussion = createModel<Percussion, PercussionWidget>("Percussion");