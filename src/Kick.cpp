#include "plugin.hpp"
#include "KickSamples.hpp"

struct Kick : Module {
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

	int numSamples = 60;

	Kick() {
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
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();
	
		const bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		const bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;
	
		if (trigRising || buttonRising) {
			kickLightBrightness = 1.f;
	
			int sampleIndex = (int)(params[SAMPLE_PARAM].getValue() + 0.5f);  // avoid rounding unless needed
			if (inputs[SAMPLECVIN_INPUT].isConnected())
				sampleIndex += (int)(inputs[SAMPLECVIN_INPUT].getVoltage());
	
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);
			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;
			playing = (currentSample && sampleLength > 1);
			env = 1.f;
		}
	
		kickLightBrightness -= args.sampleTime * 10.f;
		if (kickLightBrightness < 0.f) kickLightBrightness = 0.f;
		lights[KICK_LIGHT].setBrightness(kickLightBrightness);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			float pitchMod = params[PITCH_PARAM].getValue();
			if (inputs[PITCHCVIN_INPUT].isConnected())
				pitchMod += inputs[PITCHCVIN_INPUT].getVoltage();
	
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);
			float pitchRatio = MIN_PLAYBACK_SPEED + (pitchMod + 1.f) * 0.5f * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
	
			samplePos += pitchRatio * (44100.f / args.sampleRate);
			const int numSamps = sampleLength >> 1;  // divide by 2
	
			if ((int)samplePos >= numSamps) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamps) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				const uint8_t* s = currentSample;
				int16_t s1 = (int16_t)(s[idx * 2] | (s[idx * 2 + 1] << 8));
				int16_t s2 = (int16_t)(s[nextIdx * 2] | (s[nextIdx * 2 + 1] << 8));
	
				float sampleValue = ((float)s1 + frac * (s2 - s1)) / 32768.f;
	
				float decayParam = params[DECAY_PARAM].getValue();
				if (inputs[DECAYCVIN_INPUT].isConnected())
					decayParam += inputs[DECAYCVIN_INPUT].getVoltage();
	
				decayParam = std::clamp(decayParam, 0.f, 1.f);
				const float decayTime = 0.005f + decayParam * ((float)numSamps / 44100.f - 0.005f);
				const float decayCoef = expf(-1.f / (decayTime * args.sampleRate));
	
				env *= decayCoef;
				output = sampleValue * env;
			}
		}
	
		// Volume CV scaling
		float volume = 1.f;
		if (inputs[VOLCVIN_INPUT].isConnected())
			volume = std::clamp(inputs[VOLCVIN_INPUT].getVoltage() / 5.f, 0.f, 1.f);
	
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * volume * 5.f);
	}	
};

struct KickWidget : ModuleWidget {
	KickWidget(Kick* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Kick.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, Kick::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, Kick::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, Kick::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, Kick::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, Kick::KICK_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, Kick::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, Kick::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, Kick::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, Kick::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, Kick::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, Kick::AUDIOOUT_OUTPUT));
	}
};


Model* modelKick = createModel<Kick, KickWidget>("Kick");