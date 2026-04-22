#include "SlapSamples.hpp"
#include "cv_knob_utils.hh"
#include "plugin.hpp"
#include "sample.hh"
#include <cmath>

struct Slap : Module {
	enum ParamId { OCTAVE_PARAM, DECAY_PARAM, PUSH_PARAM, PARAMS_LEN };
	enum InputId { PITCHCVIN_INPUT, DECAYCVIN_INPUT, TRIGIN_INPUT, VOLCVIN_INPUT, OCTAVECVIN_INPUT, INPUTS_LEN };
	enum OutputId { AUDIOOUT_OUTPUT, OUTPUTS_LEN };
	enum LightId { SLAP_LIGHT, LIGHTS_LEN };

	static constexpr Sample slap{SLSlap};
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float slapLightBrightness = 0.f;

	static constexpr float sampleSampleRate = 44100.f;
	static constexpr auto numSamples = slap.size();
	static constexpr float minDecayTime = 0.005f;
	static constexpr float maxDecayTime = numSamples / sampleSampleRate;

	Slap() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configSwitch(OCTAVE_PARAM, 0.f, 4.f, 1.f, "Octave transpose", {"-1", "Unison", "+1", "+2", "+3"});
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");

		configInput(OCTAVECVIN_INPUT, "Octave CV");
		configInput(PITCHCVIN_INPUT, "Pitch CV (1V/oct)");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(TRIGIN_INPUT, "Trig");
		configInput(VOLCVIN_INPUT, "Volume CV");
		configOutput(AUDIOOUT_OUTPUT, "Audio");
	}

	void process(const ProcessArgs &args) override {
		// Trigger detection (avoid floats >1 comparisons repeatedly)
		const float trigIn = inputs[TRIGIN_INPUT].getNormalVoltage(0);
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
		if (slapLightBrightness < 0.f)
			slapLightBrightness = 0.f;
		lights[SLAP_LIGHT].setBrightnessSmooth(slapLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing) {
			float octaveKnob = params[OCTAVE_PARAM].getValue(); // 0 .. 4
			float octaveCV = inputs[OCTAVECVIN_INPUT].getNormalVoltage(0);

			// Scale CV: ±5V maps to full knob range (0..4)
			float cvScaled = octaveCV * 2.f / 5.f; // 5V → +2, -5V → -2

			// Quantize to discrete steps 0..4
			float totalOctave = std::clamp(std::round(octaveKnob + cvScaled), 0.f, 4.f);

			// standard 1V/oct, clamped to ±1V: 0V=unison, ±1V=±1oct
			float pitchCV = std::clamp(inputs[PITCHCVIN_INPUT].getNormalVoltage(0), -1.f, 1.f);
			float totalVolts = (static_cast<float>(totalOctave) - 1.f) + pitchCV;

			// Convert volts to playback rate
			// playbackRate = 2^(V) to get correct 1V/oct frequency
			float pitchRatio = calc1VperOctPitchRatio(totalVolts) * 4.f; // shift 2 octaves up

			const float sampleRateRatio = sampleSampleRate / args.sampleRate;
			samplePos += pitchRatio * sampleRateRatio;


			// Check if sample done
			if (static_cast<unsigned>(samplePos) >= numSamples) {
				playing = false;
				output = 0.f;
			} else {
				// Linear interpolate sample
				const auto idx = (uint32_t)samplePos;
				const auto nextIdx = (idx + 1 < numSamples) ? idx + 1 : idx;
				const auto frac = samplePos - idx;

				const auto s1 = slap[idx];
				const auto s2 = slap[nextIdx];

				const auto sampleValue = s1 + frac * (s2 - s1);
				const float decayCV =
					inputs[DECAYCVIN_INPUT].isConnected() ? inputs[DECAYCVIN_INPUT].getVoltage() : 0.f;
				const float decayParam = calcDecayModScaled(params[DECAY_PARAM].getValue() * 5.f, decayCV);

				const float decayTime = calcExpDecayTime(decayParam, minDecayTime, maxDecayTime);

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
			if (volumeCV < 0.f)
				volumeCV = 0.f;
			else if (volumeCV > 5.f)
				volumeCV = 5.f;
		}

		output *= volumeCV / 5.f;
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.f);
	}
};

struct SlapWidget : ModuleWidget {
	SlapWidget(Slap *module) {
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

Model *modelSlap = createModel<Slap, SlapWidget>("Slap");
