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

	int numSamples = 16;

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
			default: return nullptr;
		}
	}	

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Percussion1_len; case 1: return Percussion2_len; case 2: return Percussion3_len; case 3: return Percussion4_len;
			case 4: return Percussion5_len; case 5: return Percussion6_len; case 6: return Percussion7_len; case 7: return Percussion8_len;
			case 8: return Percussion9_len; case 9: return Percussion10_len; case 10: return Percussion11_len; case 11: return Percussion12_len;
			case 12: return Percussion13_len; case 13: return Percussion14_len; case 14: return Percussion15_len; case 15: return Percussion16_len;
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
	
			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue());
			if (inputs[SAMPLECVIN_INPUT].isConnected()) {
				// Scale CV: ±5V → ±numSamples
				float cv = inputs[SAMPLECVIN_INPUT].getVoltage();
				cv = std::clamp(cv, -5.f, 5.f);
				sampleIndex += (int)round(cv * (numSamples / 10.f)); // 10V range maps to 0–numSamples
			}
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);			
	
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
			// Pitch with ±5V CV
			float pitchMod = params[PITCH_PARAM].getValue();
			if (inputs[PITCHCVIN_INPUT].isConnected())
				pitchMod += inputs[PITCHCVIN_INPUT].getVoltage() / 5.f; // ±5V → ±1
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);
	
			float normalizedPitch = (pitchMod + 1.f) / 2.f;
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
	
			constexpr float sampleSampleRate = 44100.f;
			float sampleRateRatio = sampleSampleRate / args.sampleRate;
	
			samplePos += pitchRatio * sampleRateRatio;
	
			int numSamplesHalf = sampleLength / 2;
	
			if ((int)samplePos >= numSamplesHalf) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamplesHalf) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				int16_t s1s = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2s = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));
	
				float s1 = (float)s1s / 32768.f;
				float s2 = (float)s2s / 32768.f;
	
				float sampleValue = s1 + frac * (s2 - s1);
	
				// Decay with ±5V CV
				float decayParam = params[DECAY_PARAM].getValue();
				if (inputs[DECAYCVIN_INPUT].isConnected())
					decayParam += inputs[DECAYCVIN_INPUT].getVoltage() / 10.f; // ±5V → ±0.5
				decayParam = std::clamp(decayParam, 0.f, 1.f);
	
				float minDecayTime = 0.005f;
				float maxDecayTime = (float)numSamplesHalf / sampleSampleRate;
	
				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
				float decayCoef = expf(-1.f / (decayTime * args.sampleRate));
	
				env *= decayCoef;
	
				output = sampleValue * env;
			}
		}
	
		// Volume with ±5V CV
		float volumeCV = 10.f;
		if (inputs[VOLCVIN_INPUT].isConnected())
			volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 10.f);
	
		output *= volumeCV;
	
		outputs[AUDIOOUT_OUTPUT].setVoltage(output);
	}
};	

struct PercussionWidget : ModuleWidget {
	PercussionWidget(Percussion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Percussion.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, Percussion::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, Percussion::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, Percussion::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, Percussion::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, Percussion::PERCUSSION_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, Percussion::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, Percussion::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, Percussion::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, Percussion::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, Percussion::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, Percussion::AUDIOOUT_OUTPUT));
	}
};


Model* modelPercussion = createModel<Percussion, PercussionWidget>("Percussion");