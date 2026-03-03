#include "plugin.hpp"
#include "RideSamples.hpp"

struct SpeedQuantity : ParamQuantity {
	std::string getDisplayValueString() override {
		float v = getValue();
		float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
		return string::f("%.3gx", display);
	}
};

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

	int numSamples = 16;

	Ride() {
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
			case 0: return Ride1; case 1: return Ride2; case 2: return Ride3; case 3: return Ride4; case 4: return Ride5; case 5: return Ride6;
			case 6: return Ride7; case 7: return Ride8; case 8: return Ride9; case 9: return Ride10; case 10: return Ride11; case 11: return Ride12;
			case 12: return Ride13; case 13: return Ride14; case 14: return Ride15; case 15: return Ride16;
			default: return nullptr;
		}
	}

	int getSampleLengthByIndex(int index) {
		switch (index) {
			case 0: return Ride1_len; case 1: return Ride2_len; case 2: return Ride3_len; case 3: return Ride4_len; case 4: return Ride5_len; case 5: return Ride6_len;
			case 6: return Ride7_len; case 7: return Ride8_len; case 8: return Ride9_len; case 9: return Ride10_len; case 10: return Ride11_len; case 11: return Ride12_len;
			case 12: return Ride13_len; case 13: return Ride14_len; case 14: return Ride15_len; case 15: return Ride16_len; 
			default: return 0;
		}
	}

	void process(const ProcessArgs& args) override {
		// Cache sample time and sample rate
		const float sampleTime = args.sampleTime;
		const float sampleRate = args.sampleRate;
	
		// Trigger detection
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();
	
		const bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		const bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
	
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;
	
		if (trigRising || buttonRising) {
			RideLightBrightness = 1.f;
	
			// Sample index with ±5V CV mapping
			int sampleIndex = (int)round(params[SAMPLE_PARAM].getValue());
			if (inputs[SAMPLECVIN_INPUT].isConnected()) {
				float cv = inputs[SAMPLECVIN_INPUT].getVoltage();
				cv = std::clamp(cv, -5.f, 5.f);
				sampleIndex += (int)round(cv * (numSamples / 10.f)); // ±5V → ±numSamples/2
			}
			sampleIndex = std::clamp(sampleIndex, 0, numSamples - 1);
	
			currentSample = getSampleByIndex(sampleIndex);
			sampleLength = getSampleLengthByIndex(sampleIndex);
			samplePos = 0.f;
	
			playing = (currentSample != nullptr && sampleLength > 1);
			env = 1.f;
		}
	
		// Light decay
		RideLightBrightness -= sampleTime * 10.f;
		if (RideLightBrightness < 0.f) RideLightBrightness = 0.f;
		lights[RIDE_LIGHT].setBrightnessSmooth(RideLightBrightness, sampleTime);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			// Pitch processing (±5V CV already works like before)
		// Pitch processing with proper ±5V mapping
		float pitchMod = params[PITCH_PARAM].getValue(); // base knob
		if (inputs[PITCHCVIN_INPUT].isConnected()) {
		    float cv = std::clamp(inputs[PITCHCVIN_INPUT].getVoltage(), -5.f, 5.f);
		    pitchMod += cv / 5.f; // scale ±5V → ±1
		}
			pitchMod = std::clamp(pitchMod, -1.f, 1.f);
	
			float normalizedPitch = (pitchMod + 1.f) * 0.5f;
			const float MIN_PLAYBACK_SPEED = 0.01f;
			const float MAX_PLAYBACK_SPEED = 2.0f;
			float pitchRatio = MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
	
			constexpr float sampleSampleRate = 44100.f;
			float sampleRateRatio = sampleSampleRate / sampleRate;
	
			samplePos += pitchRatio * sampleRateRatio;
	
			const int numSamplesInSample = sampleLength / 2;
			if ((int)samplePos >= numSamplesInSample) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamplesInSample) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				int16_t s1 = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2 = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));
	
				float sampleValue = s1 * (1.f - frac) + s2 * frac;
				sampleValue /= 32768.f;
	
			// Decay with ±5V CV
			float decayParam = params[DECAY_PARAM].getValue();
			if (inputs[DECAYCVIN_INPUT].isConnected())
				decayParam += inputs[DECAYCVIN_INPUT].getVoltage() / 10.f; // ±5V → ±0.5
			decayParam = std::clamp(decayParam, 0.f, 1.f);
	
				constexpr float minDecayTime = 0.005f;
				float maxDecayTime = (float)numSamplesInSample / sampleSampleRate;
				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
	
				float decayCoef = expf(-1.f / (decayTime * sampleRate));
	
				env *= decayCoef;
	
				output = sampleValue * env;
			}
		}
	
		// Volume CV
		float volumeCV = 1.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f) / 5.f;
		}
	
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * volumeCV * 10.f);
	}	
};

struct RideWidget : ModuleWidget {
	RideWidget(Ride* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Ride.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, Ride::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, Ride::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, Ride::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, Ride::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, Ride::RIDE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, Ride::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, Ride::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, Ride::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, Ride::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, Ride::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, Ride::AUDIOOUT_OUTPUT));
	}
};


Model* modelRide = createModel<Ride, RideWidget>("Ride");