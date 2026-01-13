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

	// Use arrays to store sample pointers and lengths
	const unsigned char* samples[79] = {
		Snare1, Snare2, Snare3, Snare4, Snare5, Snare6,
		Snare7, Snare8, Snare9, Snare10, Snare11, Snare12,
		Snare13, Snare14, Snare15, Snare16, Snare17, Snare18,
		Snare19, Snare20, Snare21, Snare22, Snare23, Snare24,
		Snare25, Snare26, Snare27, Snare28, Snare29, Snare30,
		Snare31, Snare32, Snare33, Snare34, Snare35, Snare36,
		Snare37, Snare38, Snare39, Snare40, Snare41, Snare42,
		Snare43, Snare44, Snare45, Snare46, Snare47, Snare48,
		Snare49, Snare50, Snare51, Snare52, Snare53, Snare54,
		Snare55, Snare56, Snare57, Snare58, Snare59, Snare60,
		Snare61, Snare62, Snare63, Snare64, Snare65, Snare66,
		Snare67, Snare68, Snare69, Snare70, Snare71, Snare72,
		Snare73, Snare74, Snare75, Snare76, Snare77, Snare78,
		Snare79
	};

	unsigned int sampleLengths[79] = {
		Snare1_len, Snare2_len, Snare3_len, Snare4_len, Snare5_len, Snare6_len,
		Snare7_len, Snare8_len, Snare9_len, Snare10_len, Snare11_len, Snare12_len,
		Snare13_len, Snare14_len, Snare15_len, Snare16_len, Snare17_len, Snare18_len,
		Snare19_len, Snare20_len, Snare21_len, Snare22_len, Snare23_len, Snare24_len,
		Snare25_len, Snare26_len, Snare27_len, Snare28_len, Snare29_len, Snare30_len,
		Snare31_len, Snare32_len, Snare33_len, Snare34_len, Snare35_len, Snare36_len,
		Snare37_len, Snare38_len, Snare39_len, Snare40_len, Snare41_len, Snare42_len,
		Snare43_len, Snare44_len, Snare45_len, Snare46_len, Snare47_len, Snare48_len,
		Snare49_len, Snare50_len, Snare51_len, Snare52_len, Snare53_len, Snare54_len,
		Snare55_len, Snare56_len, Snare57_len, Snare58_len, Snare59_len, Snare60_len,
		Snare61_len, Snare62_len, Snare63_len, Snare64_len, Snare65_len, Snare66_len,
		Snare67_len, Snare68_len, Snare69_len, Snare70_len, Snare71_len, Snare72_len,
		Snare73_len, Snare74_len, Snare75_len, Snare76_len, Snare77_len, Snare78_len,
		Snare79_len
	};

	const float MIN_PLAYBACK_SPEED = 0.01f;
	const float MAX_PLAYBACK_SPEED = 2.0f;

	const float sampleSampleRate = 44100.f;

	const unsigned char* currentSample = nullptr;
	int sampleLength = 0; // in bytes
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float SnareLightBrightness = 0.f;

	int numSamples = 79;

	Snare() {
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

	void triggerSample(int sampleIndex) {
		sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);
		currentSample = samples[sampleIndex];
		sampleLength = sampleLengths[sampleIndex];
		samplePos = 0.f;
		playing = (currentSample != nullptr && sampleLength > 1);
		env = 1.f;
	}

	void process(const ProcessArgs& args) override {
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();
	
		const bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		const bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
	
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;
	
		if (trigRising || buttonRising) {
			SnareLightBrightness = 1.0f;
	
			// Sample CV ±5V mapping
			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue());
			if (inputs[SAMPLECVIN_INPUT].isConnected()) {
				float cv = std::clamp(inputs[SAMPLECVIN_INPUT].getVoltage(), -5.f, 5.f);
			
				// Map -5V…+5V to -numSamples…+numSamples
				int cvOffset = (int)round((cv / 5.f) * numSamples); 
				sampleIndex += cvOffset;
			}			
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);
			triggerSample(sampleIndex);
		}
	
		// Decay Snare light
		SnareLightBrightness = std::max(0.f, SnareLightBrightness - args.sampleTime * 10.f);
		lights[SNARE_LIGHT].setBrightnessSmooth(SnareLightBrightness, args.sampleTime);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			// Pitch CV ±5V mapping
			float pitchMod = params[PITCH_PARAM].getValue();
			if (inputs[PITCHCVIN_INPUT].isConnected()) {
				float cv = std::clamp(inputs[PITCHCVIN_INPUT].getVoltage(), -5.f, 5.f);
				pitchMod += cv / 5.f; // scale ±5V → ±1
			}
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);
	
			float normalizedPitch = 0.5f * (pitchMod + 1.f);
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
	
			const float sampleRateRatio = sampleSampleRate / args.sampleRate;
			samplePos += pitchRatio * sampleRateRatio;
	
			const int numSampleFrames = sampleLength / 2;
	
			if ((int)samplePos >= numSampleFrames) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSampleFrames) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				int16_t s1s = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2s = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));
	
				float s1 = s1s / 32768.f;
				float s2 = s2s / 32768.f;
	
				float sampleValue = s1 + frac * (s2 - s1);
	
				// Decay CV ±5V mapping
				float decayParam = params[DECAY_PARAM].getValue();
				if (inputs[DECAYCVIN_INPUT].isConnected()) {
					float cv = std::clamp(inputs[DECAYCVIN_INPUT].getVoltage(), -5.f, 5.f);
					decayParam += cv / 5.f; // ±5V → ±1
				}
				decayParam = std::clamp(decayParam, 0.f, 1.f);
	
				const float minDecayTime = 0.005f;
				float maxDecayTime = (float)numSampleFrames / sampleSampleRate;
				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
	
				float decayCoef = expf(-1.f / (decayTime * args.sampleRate));
				env *= decayCoef;
	
				output = sampleValue * env;
			}
		}
	
		// Volume CV (0–5V)
		float volumeCV = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
		}
	
		output *= volumeCV / 5.f;
	
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);
	}	
};

struct SnareWidget : ModuleWidget {
	SnareWidget(Snare* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Snare.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, Snare::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, Snare::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, Snare::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, Snare::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, Snare::SNARE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, Snare::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, Snare::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, Snare::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, Snare::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, Snare::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, Snare::AUDIOOUT_OUTPUT));
	}
};


Model* modelSnare = createModel<Snare, SnareWidget>("Snare");