#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>

template<size_t NumSamples>
constexpr int indexed_cv_knob(float cv, float knob) {
	float sampleKnobSel = std::clamp(knob / NumSamples, 0.f, 1.f);
	float sampleCV = std::clamp(cv / 5.f, -1.f, 1.f);
	int sampleIndex = std::round((sampleCV + sampleKnobSel) * NumSamples);
	return std::clamp<int>(sampleIndex, 0, NumSamples - 1);
}

template<float MIN_PLAYBACK_SPEED, float MAX_PLAYBACK_SPEED>
constexpr float pitch_cv_knob(float cv, float knob) {
	float pitchCV = cv / 5.f;
	float pitchMod = std::clamp(knob + pitchCV, -1.f, 1.f);
	float normalizedPitch = (pitchMod + 1.f) * 0.5f;
	return MIN_PLAYBACK_SPEED + normalizedPitch * (MAX_PLAYBACK_SPEED - MIN_PLAYBACK_SPEED);
}
