#pragma once

#include "cv_knob_utils.hh"
#include "plugin.hpp"
#include "sample.hh"

struct NamedSample {
	const char *name;
	Sample sample;
	float scale = 1.f;
};

template<typename T>
class DrumMachineBaseModule : public Module {
	struct Voice {
		bool lastInputTrigger = false;
		bool lastButtonTrigger = false;
		float samplePos = 0.f;
		bool playing = false;
	};

public:
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		MAINVOL_PARAM,
		DRUM0_PARAM,
	};

	static constexpr unsigned PARAMS_LEN = T::drums.size() + 3;

	enum InputId {
		SPEEDCVIN_INPUT,
		LENGTHCVIN_INPUT,
		DRUM0_INPUT,
	};

	static constexpr unsigned INPUTS_LEN = T::drums.size() + 2;

	enum OutputId {
		DRUM0_OUTPUT,
	};

	static constexpr unsigned SUM_OUTPUT = T::drums.size();
	static constexpr unsigned OUTPUTS_LEN = T::drums.size() + 1;

	enum LightId {
		DRUM0_LIGHT,
	};

	static constexpr unsigned LIGHTS_LEN = T::drums.size();

	static constexpr float LENGTH_MIN = 0.1f;
	static constexpr float LENGTH_MAX = 1.0f;

	// Display-side: shows the actual playback rate produced by pitch_cv_knob,
	// including any voltage on SPEEDCVIN_INPUT, so the readout matches what's heard.
	struct SpeedQuantity : ParamQuantity {
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

	std::array<Voice, 12> voice{};

	DrumMachineBaseModule() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed", "x");
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configParam(MAINVOL_PARAM, 0.f, 1.f, 0.5f, "Main Volume", "%", 0.f, 100.f);

		for (auto i = 0u; i < T::drums.size(); ++i) {
			std::string v = std::string{T::drums[i].name} + " Trig";
			configSwitch(i + DRUM0_PARAM, 0.f, 1.f, 0.f, v, {"Off", "On"});
			configInput(i + DRUM0_INPUT, v);
			configOutput(i + DRUM0_OUTPUT, T::drums[i].name);
		}

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configOutput(SUM_OUTPUT, "Sum");
	}

	void process(const ProcessArgs &args) override {
		// --- Speed ---
		float pitchKnob = params[SPEED_PARAM].getValue(); // -1 to 1
		float pitchCV = inputs[SPEEDCVIN_INPUT].getNormalVoltage(0);
		float pitchRatio = pitch_cv_knob(pitchCV, pitchKnob);

		// --- Length ---
		const auto lengthCV = inputs[LENGTHCVIN_INPUT].getNormalVoltage(0);
		const auto normLength = std::clamp(params[LENGTH_PARAM].getValue() + (lengthCV * 0.1f), 0.1f, 1.0f);
		const auto lengthRatio = LENGTH_MIN + normLength * (LENGTH_MAX - LENGTH_MIN);

		const auto mainVol = params[MAINVOL_PARAM].getValue();

		float busSum = 0.f;

		for (auto i = 0u; i < T::drums.size(); ++i) {
			const auto output_id = i + DRUM0_OUTPUT;
			const auto input_id = i + DRUM0_INPUT;
			const auto param_id = i + DRUM0_PARAM;
			const auto light_id = i + DRUM0_LIGHT;
			const Sample &samp = T::drums[i].sample;

			auto &v = voice[i];

			bool inTrig = inputs[input_id].getVoltage() > 1.f;
			bool btnTrig = params[param_id].getValue() > 0.5f;
			bool inputRising = inTrig && !v.lastInputTrigger;
			bool buttonRising = btnTrig && !v.lastButtonTrigger;

			bool fired = inputRising || buttonRising;

			if (fired) {
				v.playing = true;
				v.samplePos = 0.f;
			}

			v.lastInputTrigger = inTrig;
			v.lastButtonTrigger = btnTrig;

			const auto maxSamples = static_cast<unsigned>(samp.size() * lengthRatio);

			if (v.playing) {
				const auto idx = static_cast<uint32_t>(v.samplePos);
				if (idx < maxSamples) {
					const auto sumVoltage = samp[idx] * 5.f * T::drums[i].scale;

					if (outputs[output_id].isConnected()) {
						outputs[output_id].setVoltage(sumVoltage);
					} else {
						busSum += sumVoltage;
					}

					v.samplePos += pitchRatio;
				} else {
					v.playing = false;
					outputs[output_id].setVoltage(0.f);
				}
			} else {
				outputs[output_id].setVoltage(0.f);
			}

			if (fired)
				lights[light_id].setBrightness(1.f);
			else
				lights[light_id].setBrightnessSmooth(0.f, args.sampleTime);
		}

		outputs[SUM_OUTPUT].setVoltage(std::clamp(busSum * mainVol, -10.f, 10.f));
	}
};

// Single-column drum layout. Drums fill {button, input, output} columns evenly spaced.
// Top row (Length/Speed/MainVol + CV/Sum) uses larger knobs for <=4 drums
// and a compact layout otherwise.
template<typename T, typename W>
class DrumMachineVerticalBase : public ModuleWidget {
public:
	DrumMachineVerticalBase(W *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, T::panel)));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		constexpr size_t numDrums = W::drums.size();
		constexpr bool chunkyTop = numDrums <= 4;

		if constexpr (chunkyTop) {
			addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(9.751, 29.2)), module, W::LENGTH_PARAM));
			addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 19.001)), module, W::SPEED_PARAM));
			addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(41.25, 29.2)), module, W::MAINVOL_PARAM));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.751, 47.001)), module, W::LENGTHCVIN_INPUT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 36.499)), module, W::SPEEDCVIN_INPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(41.25, 47.001)), module, W::SUM_OUTPUT));
		} else {
			addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module, W::LENGTH_PARAM));
			addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module, W::SPEED_PARAM));
			addParam(createParamCentered<Knob9mm>(mm2px(Vec(44.2, 12.45)), module, W::MAINVOL_PARAM));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module, W::LENGTHCVIN_INPUT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module, W::SPEEDCVIN_INPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module, W::SUM_OUTPUT));
		}

		constexpr float yStart = chunkyTop ? 67.f : 37.f;
		constexpr float yEnd = 112.f;
		constexpr float ySpacing = numDrums > 1 ? (yEnd - yStart) / (numDrums - 1) : 0.f;
		constexpr float xButton = 7.751f;
		constexpr float xInput = 32.0f;
		constexpr float xOutput = 43.998f;

		for (size_t i = 0; i < numDrums; ++i) {
			const float y = yStart + ySpacing * i;
			addParam(createParamCentered<LEDBezel>(mm2px(Vec(xButton, y)), module, W::DRUM0_PARAM + i));
			addChild(
				createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(xButton, y)), module, W::DRUM0_LIGHT + i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xInput, y)), module, W::DRUM0_INPUT + i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xOutput, y)), module, W::DRUM0_OUTPUT + i));
		}
	}
};

struct DrumGridLayout {
	std::array<float, 6> colX{}; // unused entries left as 0
	size_t cols = 0;
	float topX0 = 0.f;
	float topX1 = 0.f;
	float topX2 = 0.f;
};

inline constexpr DrumGridLayout k12DrumLayout{
	.colX = {8.798f, 20.5f, 32.3f, 43.998f, 55.7f, 67.501f},
	.cols = 6,
	.topX0 = 16.799f,
	.topX1 = 38.799f,
	.topX2 = 59.351f,
};
inline constexpr DrumGridLayout k10DrumLayout{
	.colX = {6.999f, 18.75f, 30.501f, 42.249f, 54.0f},
	.cols = 5,
	.topX0 = 11.24f,
	.topX1 = 31.249f,
	.topX2 = 49.731f,
};

// Two-row grid layout used by 10/12-drum machines. Top row holds Length, Speed,
// MainVol knobs (with Length/Speed CV inputs and Sum output below). Drums fill
// L.cols columns × 2 rows; column x positions come from the layout struct.
template<typename T, typename W, DrumGridLayout L>
class DrumMachineGridBase : public ModuleWidget {
public:
	DrumMachineGridBase(W *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, T::panel)));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(L.topX0, 15.501)), module, W::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(L.topX1, 15.501)), module, W::SPEED_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(L.topX2, 15.501)), module, W::MAINVOL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(L.topX0, 30.801)), module, W::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(L.topX1, 30.801)), module, W::SPEEDCVIN_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(L.topX2, 30.801)), module, W::SUM_OUTPUT));

		constexpr float yButtonRow[2] = {42.002f, 84.999f};
		constexpr float yInputRow[2] = {56.0f, 99.001f};
		constexpr float yOutputRow[2] = {70.002f, 112.999f};

		for (size_t i = 0; i < W::drums.size(); ++i) {
			const size_t row = i / L.cols;
			const float x = L.colX[i % L.cols];
			addParam(createParamCentered<LEDBezel>(mm2px(Vec(x, yButtonRow[row])), module, W::DRUM0_PARAM + i));
			addChild(createLightCentered<LEDBezelLight<WhiteLight>>(
				mm2px(Vec(x, yButtonRow[row])), module, W::DRUM0_LIGHT + i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x, yInputRow[row])), module, W::DRUM0_INPUT + i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x, yOutputRow[row])), module, W::DRUM0_OUTPUT + i));
		}
	}
};

template<typename T, typename W>
struct DrumMachine12BaseWidget : DrumMachineGridBase<T, W, k12DrumLayout> {
	using DrumMachineGridBase<T, W, k12DrumLayout>::DrumMachineGridBase;
};

template<typename T, typename W>
struct DrumMachine10BaseWidget : DrumMachineGridBase<T, W, k10DrumLayout> {
	using DrumMachineGridBase<T, W, k10DrumLayout>::DrumMachineGridBase;
};
