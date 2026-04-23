#pragma once
#include "cv_knob_utils.hh"
#include "plugin.hpp"

template<typename T>
class OneShotBaseModule : public Module {
public:
	enum ParamId { SAMPLE_PARAM, PITCH_PARAM, DECAY_PARAM, PUSH_PARAM, PARAMS_LEN };
	enum InputId { SAMPLECVIN_INPUT, PITCHCVIN_INPUT, DECAYCVIN_INPUT, TRIGIN_INPUT, VOLCVIN_INPUT, INPUTS_LEN };
	enum OutputId { AUDIOOUT_OUTPUT, OUTPUTS_LEN };
	enum LightId { LIGHT, LIGHTS_LEN };

	float samplePos = 0.f;
	bool playing = false;

	uint32_t cur_sample_idx{};

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float lightBrightness = 0.f;

	float decayCoef = 1.f;
	float maxDecayTime = 0.f; // set at trigger (depends on sample length)

	float last_decayKnob = 0.f;
	float last_decayCV = 0.f;
	float last_sampleRate = 0.f;

	static constexpr size_t NumSamples = T::samples.size();

	// Display-side: shows the actual playback rate produced by pitch_cv_knob,
	// including any voltage on PITCHCVIN_INPUT, so the readout matches what's heard.
	struct PitchQuantity : ParamQuantity {
		float getDisplayValue() override {
			if (!module)
				return ParamQuantity::getDisplayValue();
			const float cv = 0;
			return pitch_cv_knob(cv, getValue());
		}
		void setDisplayValue(float displayValue) override {
			setValue(pitch_knob_from_rate(displayValue));
		}
	};

	OneShotBaseModule() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		std::vector<std::string> sampleChoices;
		for (auto i = 1u; i <= NumSamples; ++i)
			sampleChoices.push_back(std::to_string(i));
		configSwitch(SAMPLE_PARAM, 0.f, (NumSamples - 1), 0.f, "Sample", sampleChoices);
		configParam<PitchQuantity>(PITCH_PARAM, -1.f, 1.f, 0.f, "Pitch", "x");
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");
		configInput(SAMPLECVIN_INPUT, "Sample CV");
		configInput(PITCHCVIN_INPUT, "Pitch CV");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(VOLCVIN_INPUT, "Volume CV");
		configInput(TRIGIN_INPUT, "Trig");
		configOutput(AUDIOOUT_OUTPUT, "Audio output");
	}

	void process(const ProcessArgs &args) override {
		float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		float buttonIn = params[PUSH_PARAM].getValue();

		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		bool triggered = trigRising || buttonRising;

		if (triggered) {
			lightBrightness = 1.0f;

			cur_sample_idx = indexed_cv_knob<NumSamples>(inputs[SAMPLECVIN_INPUT].getNormalVoltage(0),
														 params[SAMPLE_PARAM].getValue());

			samplePos = 0.f;
			playing = true;
			env = 1.0f;

			// Sample-length-dependent max decay only changes on trigger.
			maxDecayTime = T::samples[cur_sample_idx].size() / 44100.f;
		}

		lightBrightness = std::max(0.f, lightBrightness - (float)(args.sampleTime * 10.f));
		lights[LIGHT].setBrightnessSmooth(lightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing) {
			// Recompute only when inputs move by more than kChangeThresh, to
			// avoid per-sample expf/pow (expensive on ARM targets).
			const float pitchKnob = params[PITCH_PARAM].getValue();
			const float pitchCV = inputs[PITCHCVIN_INPUT].getNormalVoltage(0);
			const float decayKnob = params[DECAY_PARAM].getValue();
			const float decayCV = inputs[DECAYCVIN_INPUT].getNormalVoltage(0);
			const float sr = args.sampleRate;

			const bool srChanged = changed(sr, last_sampleRate);
			const bool decayChanged = changed(decayKnob, last_decayKnob) || changed(decayCV, last_decayCV);

			if (triggered || decayChanged || srChanged) {
				static constexpr float minDecayTime = 0.005f;
				const float decayMod = calcDecayMod(decayKnob, decayCV);
				const float decayTime = minDecayTime + decayMod * (maxDecayTime - minDecayTime);
				decayCoef = expf(-1.f / (decayTime * sr));
				last_decayKnob = decayKnob;
				last_decayCV = decayCV;
				last_sampleRate = sr;
			}

			// Sample playback
			const float pitchRatio = pitch_cv_knob(pitchCV, pitchKnob);
			samplePos += pitchRatio * 44100.f / sr;

			if ((uint32_t)samplePos >= T::samples[cur_sample_idx].size()) {
				playing = false;
			} else {
				auto idx = (uint32_t)samplePos;
				auto nextIdx = (idx + 1 < T::samples[cur_sample_idx].size()) ? idx + 1 : idx;
				auto frac = samplePos - idx;
				auto sampleValue = T::samples[cur_sample_idx][idx] +
								   frac * (T::samples[cur_sample_idx][nextIdx] - T::samples[cur_sample_idx][idx]);
				env *= decayCoef;
				if (decayCoef <= 1e-6f)
					playing = false;
				output = sampleValue * env;
			}
		}

		float volumeCV = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = std::clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
		}

		output *= volumeCV / 5.f;

		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 10.0f);
	}
};

template<typename T, typename M>
class OneShotBaseWidget : public ModuleWidget {
public:
	OneShotBaseWidget(M *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, T::panel)));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 12.45)), module, M::SAMPLE_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 36.199)), module, M::PITCH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(10.16, 60.001)), module, M::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 84.3)), module, M::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 84.3)), module, M::LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.15)), module, M::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 49.001)), module, M::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 72.701)), module, M::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.5, 98.002)), module, M::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.799, 98.002)), module, M::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 112.0)), module, M::AUDIOOUT_OUTPUT));
	}
};
