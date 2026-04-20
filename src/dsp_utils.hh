#pragma once
#include <algorithm>

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

// KayOne length: knob (0..1) added to lightly-scaled CV (±10V * 0.1), hard floor at 0.1
// Returns length ratio 0.1..1.0 directly (not a 0..1 intermediate)
inline float calcLengthMod(float knobValue, float cvVoltage) {
    return std::clamp(knobValue + cvVoltage * 0.1f, 0.1f, 1.0f);
}
