#include "plugin.hpp"
#include "SlapSamples.hpp"
#include <cmath>

struct Slap : Module {
	enum ParamId {
		OCTAVE_PARAM,
		DECAY_PARAM,
		PUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PITCHCVIN_INPUT,
		DECAYCVIN_INPUT,
		TRIGIN_INPUT,
		VOLCVIN_INPUT,
		OCTAVECVIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		SLAP_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = Slap1;
	int sampleLength = Slap1_len;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float slapLightBrightness = 0.f;

	static constexpr float sampleSampleRate = 44100.f;

	Slap() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configSwitch(OCTAVE_PARAM, 0.f, 4.f, 0.f, "Octave transpose", {"Unison", "+1", "+2", "+3", "+4"});
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");

		configInput(OCTAVECVIN_INPUT, "Octave CV");
		configInput(PITCHCVIN_INPUT, "Pitch CV (1V/oct)");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(TRIGIN_INPUT, "Trig");
		configInput(VOLCVIN_INPUT, "Volume CV");
		configOutput(AUDIOOUT_OUTPUT, "Audio");
	}

	float fastPow2(float x) {
		// Approximate 2^x for typical pitch range (x in [0..8])
		// Using a simple polynomial or linear approx to avoid std::pow cost
		// For small CPU optimization, linear approx is OK here:
		// 2^x ≈ 1 + x * 0.69314718 (ln2) for x near 0, std::clamp for larger x.
		// Since pitch can be 0..8 (4 oct + 4V?), std::clamp and do powf only if needed.

		if (x < 0.f) return 1.f / fastPow2(-x);
		if (x > 8.f) x = 8.f;
		return 1.f + x * 0.69314718f;
	}

	void process(const ProcessArgs& args) override {
		// Trigger detection (avoid floats >1 comparisons repeatedly)
		const float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		const float buttonIn = params[PUSH_PARAM].getValue();

		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		if (trigRising || buttonRising) {
			slapLightBrightness = 1.f;
			samplePos = 0.f;
			playing = true;
			env = 1.f;
		}

		// Update light with smooth fade, decrement by fixed rate
		slapLightBrightness -= args.sampleTime * 10.f;
		if (slapLightBrightness < 0.f) slapLightBrightness = 0.f;
		lights[SLAP_LIGHT].setBrightnessSmooth(slapLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSample) {
			// Pitch calculation: knob + CV, std::clamp CV if disconnected
			// --- Octave parameter with CV scaling ---
// --- 1V/oct pitch calculation ---
int finalOctave = std::clamp(static_cast<int>(params[OCTAVE_PARAM].getValue()) + static_cast<int>(std::round(std::clamp(inputs[OCTAVECVIN_INPUT].getVoltage(), -5.f, 5.f) * 0.4f)), 0, 4);
float pitchCV  = inputs[PITCHCVIN_INPUT].isConnected() ? inputs[PITCHCVIN_INPUT].getVoltage() : 0.f;

// Sum everything in volts for 1V/oct tracking
// Each volt = 1 octave, so total pitch in volts:
float totalVolts = finalOctave + pitchCV;

// Convert volts to playback rate
// playbackRate = 2^(V) to get correct 1V/oct frequency
float pitchRatio = std::pow(2.f, totalVolts);


			const float sampleRateRatio = sampleSampleRate / args.sampleRate;
			samplePos += pitchRatio * sampleRateRatio;

			const int numSamples = sampleLength / 2;

			// Check if sample done
			if ((int)samplePos >= numSamples) {
				playing = false;
				output = 0.f;
			} else {
				// Linear interpolate sample
				const int idx = (int)samplePos;
				const int nextIdx = (idx + 1 < numSamples) ? idx + 1 : idx;
				const float frac = samplePos - idx;

				// Read sample data once
				const unsigned char* d = currentSample;
				const int16_t s1s = (int16_t)(d[idx * 2] | (d[idx * 2 + 1] << 8));
				const int16_t s2s = (int16_t)(d[nextIdx * 2] | (d[nextIdx * 2 + 1] << 8));
				const float s1 = s1s * (1.f / 32768.f);
				const float s2 = s2s * (1.f / 32768.f);

				const float sampleValue = s1 + frac * (s2 - s1);
		// --- Decay parameter with 0-5 knob + -10..10 CV ---
			float decayKnob = params[DECAY_PARAM].getValue() * 5.f;  // 0-1 → 0-5
			float decayCV = 0.f;
			if (inputs[DECAYCVIN_INPUT].isConnected()) {
			    decayCV = inputs[DECAYCVIN_INPUT].getVoltage() * 0.5f;  // -10..10 → -5..5
			}
			float decayParam = decayKnob + decayCV;        // sum knob + CV
			decayParam = std::clamp(decayParam, 0.f, 5.f); 
			decayParam /= 5.f;                             // normalize back to 0-1

			const float minDecayTime = 0.005f;
			const float maxDecayTime = numSamples / sampleSampleRate;
			const float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);

			// Calculate decay coefficient once per sample
			const float decayCoef = expf(-1.f / (decayTime * args.sampleRate));
			env *= decayCoef;

				output = sampleValue * env;
			}
		}

		// Volume CV (std::clamped 0..5V)
		float volumeCV = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = inputs[VOLCVIN_INPUT].getVoltage();
			if (volumeCV < 0.f) volumeCV = 0.f;
			else if (volumeCV > 5.f) volumeCV = 5.f;
		}

		output *= volumeCV / 5.f;
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.f);
	}
};


struct SlapWidget : ModuleWidget {
	SlapWidget(Slap* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Slap.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, Slap::OCTAVE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 55.002)), module, Slap::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, Slap::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, Slap::SLAP_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, Slap::OCTAVECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 38.199)), module, Slap::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 67.702)), module, Slap::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, Slap::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, Slap::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, Slap::AUDIOOUT_OUTPUT));
	}
};


Model* modelSlap = createModel<Slap, SlapWidget>("Slap");