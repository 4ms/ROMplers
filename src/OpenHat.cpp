#include "plugin.hpp"
#include "OpenHatSamples.hpp"

struct SpeedQuantity : ParamQuantity {
	std::string getDisplayValueString() override {
		float v = getValue();
		float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
		return string::f("%.3gx", display);
	}
};

struct OpenHat : Module {
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
		OPENHAT_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float OpenHatLightBrightness = 0.f;

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 8.0f;

	int numSamples = 16;

	OpenHat() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		std::vector<std::string> sampleChoices;
		for (int i = 1; i <= numSamples; ++i)
			sampleChoices.push_back(std::to_string(i));
		configSwitch(SAMPLE_PARAM, 0.f, (numSamples-1), 0.f, "Sample", sampleChoices);
		configParam<SpeedQuantity>(PITCH_PARAM, -1.f, 1.f, 0.f, "Pitch");
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
			case 0: return OpenHat1; case 1: return OpenHat2; case 2: return OpenHat3; case 3: return OpenHat4; case 4: return OpenHat5; case 5: return OpenHat6;
			case 6: return OpenHat7; case 7: return OpenHat8; case 8: return OpenHat9; case 9: return OpenHat10; case 10: return OpenHat11; case 11: return OpenHat12;
			case 12: return OpenHat13; case 13: return OpenHat14; case 14: return OpenHat15; case 15: return OpenHat16; case 16: 
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return OpenHat1_len; case 1: return OpenHat2_len; case 2: return OpenHat3_len; case 3: return OpenHat4_len; case 4: return OpenHat5_len; case 5: return OpenHat6_len;
			case 6: return OpenHat7_len; case 7: return OpenHat8_len; case 8: return OpenHat9_len; case 9: return OpenHat10_len; case 10: return OpenHat11_len; case 11: return OpenHat12_len;
			case 12: return OpenHat13_len; case 13: return OpenHat14_len; case 14: return OpenHat15_len; case 15: return OpenHat16_len; 
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
			OpenHatLightBrightness = 1.f;
	
			// --- SAMPLE INDEX WITH CV ---
			float sampleKnob = params[SAMPLE_PARAM].getValue(); // 0..numSamples-1
			float sampleCV = inputs[SAMPLECVIN_INPUT].isConnected() ? inputs[SAMPLECVIN_INPUT].getVoltage() / 5.f : 0.f; // -1..1
			sampleCV = std::clamp(sampleCV, -1.f, 1.f);
	
			float sampleMod = sampleKnob + ((sampleCV > 0 ? (numSamples - 1 - sampleKnob) : sampleKnob) * sampleCV);
			int sampleIndex = std::clamp((int)round(sampleMod), 0, numSamples - 1);
	
			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;
			playing = (currentSample && sampleLength > 1);
			env = 1.f;
		}
	
		// --- LIGHT DECAY ---
		OpenHatLightBrightness -= args.sampleTime * 10.f;
		if (OpenHatLightBrightness < 0.f) OpenHatLightBrightness = 0.f;
		lights[OPENHAT_LIGHT].setBrightness(OpenHatLightBrightness);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			const int numSamps = sampleLength >> 1;
	
			// --- PITCH WITH CV ---
			float pitchKnob = params[PITCH_PARAM].getValue(); // -1..1
			float pitchCV = inputs[PITCHCVIN_INPUT].isConnected() ? inputs[PITCHCVIN_INPUT].getVoltage() / 5.f : 0.f; // -1..1
			pitchCV = std::clamp(pitchCV, -1.f, 1.f);
	
			float pitchMod = pitchKnob + ((pitchCV > 0 ? (1.f - pitchKnob) : (1.f + pitchKnob)) * pitchCV);
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);
	
			float pitchRatio = MIN_PLAYBACK_SPEED + (pitchMod + 1.f) * 0.5f * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
			const float sampleRateRatio = 44100.f / args.sampleRate;
			samplePos += pitchRatio * sampleRateRatio;
	
			int idx = (int)samplePos;
			if (idx >= numSamps) {
				playing = false;
			} else {
				int nextIdx = (idx + 1 < numSamps) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				const uint8_t* s = currentSample;
				int16_t s1 = (int16_t)(s[idx * 2] | (s[idx * 2 + 1] << 8));
				int16_t s2 = (int16_t)(s[nextIdx * 2] | (s[nextIdx * 2 + 1] << 8));
				float sampleValue = ((float)s1 + frac * (s2 - s1)) / 32768.f;
	
				// --- DECAY WITH CV ---
				float decayKnob = params[DECAY_PARAM].getValue(); // 0..1
				float decayCV = inputs[DECAYCVIN_INPUT].isConnected() ? inputs[DECAYCVIN_INPUT].getVoltage() / 5.f : 0.f; // -1..1
				decayCV = std::clamp(decayCV, -1.f, 1.f);
	
				float decayMod = decayKnob + ((decayCV > 0 ? (1.f - decayKnob) : decayKnob) * decayCV);
				decayMod = std::clamp(decayMod, 0.f, 1.f);
	
				const float decayTime = 0.005f + decayMod * ((float)numSamps / 44100.f - 0.005f);
				const float decayCoef = expf(-1.f / (decayTime * args.sampleRate));
	
				env *= decayCoef;
				output = sampleValue * env;
			}
		}
	
		// --- VOLUME CV ---
		float volume = inputs[VOLCVIN_INPUT].isConnected() ? std::clamp(inputs[VOLCVIN_INPUT].getVoltage() / 5.f, 0.f, 1.f) : 1.f;
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * volume * 10.f);
	}
};	

struct OpenHatWidget : ModuleWidget {
	OpenHatWidget(OpenHat* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/OpenHat.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, OpenHat::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, OpenHat::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, OpenHat::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, OpenHat::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, OpenHat::OPENHAT_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, OpenHat::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, OpenHat::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, OpenHat::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, OpenHat::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, OpenHat::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, OpenHat::AUDIOOUT_OUTPUT));
	}
};


Model* modelOpenHat = createModel<OpenHat, OpenHatWidget>("OpenHat");