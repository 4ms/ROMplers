#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>

// Threshold for detecting meaningful knob/CV movement — 1/1000 of the raw unit.
// Used to skip expensive recomputes (expf/pow) when inputs are static.
inline constexpr float kChangeThresh = 1e-3f;
inline bool changed(float a, float b) {
	return std::abs(a - b) > kChangeThresh;
}

// Knob (0..1) rescaled to -5..5V offset; CV added and clamped to ±5V; rescaled to 0..1
inline float calcDecayMod(float knobValue, float cvVoltage) {
	const float knobOffset = knobValue * 10.f - 5.f;
	return (std::clamp(knobOffset + cvVoltage, -5.f, 5.f) + 5.f) * 0.1f;
}

// Knob (0..5 range) added to CV (±10V scaled by 0.5); result clamped to 0..1
// Used by OrchHits (knob configured 0..5 directly) and Slap (knob 0..1, caller scales by 5)
inline float calcDecayModScaled(float knobValue, float cvVoltage) {
	return std::clamp((knobValue + cvVoltage * 0.5f) / 5.f, 0.f, 1.f);
}

// Exponential (logarithmic) decay time: param 0..1 maps to minTime..maxTime
// on a log scale so each half of the knob spans the same number of doublings.
inline float calcExpDecayTime(float param, float minTime, float maxTime) {
	return minTime * std::pow(maxTime / minTime, param);
}

// 1V/oct pitch ratio for Slap/OrchHits: each volt = one octave.
// totalVolts = (octaveKnobIndex - unisonIndex) + pitchCV
inline float calc1VperOctPitchRatio(float totalVolts) {
	return std::pow(2.f, totalVolts);
}

// Piecewise linear pitch mapping from normalized position (0..1) to playback ratio:
// 0.0 -> 0.01x, 0.5 -> 1.0x, 1.0 -> 2.0x
inline float calcPitchRatio(float normalizedPitch) {
	if (normalizedPitch <= 0.5f)
		return 0.01f + normalizedPitch * 1.98f;
	else
		return 1.0f + (normalizedPitch - 0.5f) * 2.0f;
}

template<size_t NumSamples>
constexpr int indexed_cv_knob(float cv, float knob) {
	float sampleKnobSel = std::clamp(knob / NumSamples, 0.f, 1.f);
	float sampleCV = std::clamp(cv / 10.f, -1.f, 1.f);
	int sampleIndex = std::round((sampleCV + sampleKnobSel) * NumSamples);
	return std::clamp<int>(sampleIndex, 0, NumSamples - 1);
}

template<typename T>
constexpr float pitch_cv_knob(float cv, float knob) {
	float pitchCV = cv / 5.f;
	float pitchMod = std::clamp(knob + pitchCV, -1.f, 1.f);
	float normalizedPitch = (pitchMod + 1.f) * 0.5f;
	return T::min_rate + normalizedPitch * (T::max_rate - T::min_rate);
}
