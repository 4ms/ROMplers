#include <doctest/doctest.h>
#include "dsp_utils.hh"
#include <cmath>
#include <algorithm>

// Slap/OrchHits pitch CV: standard 1V/oct, clamped to ±5V.
// 0V=unison, +1V=+1oct, -1V=-1oct, +1/12V=one semitone up.
// Octave transpose knob/CV applies after.
// Slap:     unisonIndex=1, octave knob 0..4 = {-1oct, unison, +1, +2, +3}
// OrchHits: unisonIndex=2, octave knob 0..4 = {-2oct, -1oct, unison, +1, +2}

static float rescalePitchCV(float inputV) {
    return std::clamp(inputV, -1.f, 1.f);
}

static float slapTotalVolts(float octaveIndex, float inputV) {
    return (octaveIndex - 1.f) + rescalePitchCV(inputV);
}

static float orchTotalVolts(float octaveIndex, float inputV) {
    return (octaveIndex - 2.f) + rescalePitchCV(inputV);
}

// --- CV clamping ---

TEST_CASE("CV: 0V -> 0V (unison)") {
    CHECK(rescalePitchCV(0.f) == doctest::Approx(0.f));
}

TEST_CASE("CV: +1V -> +1V (one octave up)") {
    CHECK(rescalePitchCV(1.f) == doctest::Approx(1.f));
}

TEST_CASE("CV: -1V -> -1V (one octave down)") {
    CHECK(rescalePitchCV(-1.f) == doctest::Approx(-1.f));
}

TEST_CASE("CV: above +1V clamped to +1V") {
    CHECK(rescalePitchCV(5.f) == doctest::Approx(1.f));
    CHECK(rescalePitchCV(10.f) == doctest::Approx(1.f));
}

TEST_CASE("CV: below -1V clamped to -1V") {
    CHECK(rescalePitchCV(-5.f) == doctest::Approx(-1.f));
    CHECK(rescalePitchCV(-10.f) == doctest::Approx(-1.f));
}

// --- 1V/oct pitch ratios ---

TEST_CASE("1V/oct: 0V -> 1.0x (unison)") {
    CHECK(calc1VperOctPitchRatio(0.f) == doctest::Approx(1.0f));
}

TEST_CASE("1V/oct: +1V -> 2.0x (one octave up)") {
    CHECK(calc1VperOctPitchRatio(1.f) == doctest::Approx(2.0f));
}

TEST_CASE("1V/oct: -1V -> 0.5x (one octave down)") {
    CHECK(calc1VperOctPitchRatio(-1.f) == doctest::Approx(0.5f));
}

TEST_CASE("1V/oct: 1/12 V -> one semitone up") {
    CHECK(calc1VperOctPitchRatio(1.f / 12.f) == doctest::Approx(std::pow(2.f, 1.f / 12.f)));
}

TEST_CASE("1V/oct: -1/12 V -> one semitone down") {
    CHECK(calc1VperOctPitchRatio(-1.f / 12.f) == doctest::Approx(std::pow(2.f, -1.f / 12.f)));
}

// --- Slap (unisonIndex=1) ---

TEST_CASE("Slap: Unison(1), 0V -> 1.0x") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(1.f, 0.f)) == doctest::Approx(1.0f));
}

TEST_CASE("Slap: Unison(1), +1V -> 2.0x") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(1.f, 1.f)) == doctest::Approx(2.0f));
}

TEST_CASE("Slap: Unison(1), -1V -> 0.5x") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(1.f, -1.f)) == doctest::Approx(0.5f));
}

TEST_CASE("Slap: Unison(1), 1/12 V -> one semitone up") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(1.f, 1.f / 12.f)) ==
          doctest::Approx(std::pow(2.f, 1.f / 12.f)));
}

TEST_CASE("Slap: -1oct(0), 0V -> 0.5x") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(0.f, 0.f)) == doctest::Approx(0.5f));
}

TEST_CASE("Slap: +1oct(2), 0V -> 2.0x") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(2.f, 0.f)) == doctest::Approx(2.0f));
}

TEST_CASE("Slap: +1oct(2), +1V -> 4.0x") {
    CHECK(calc1VperOctPitchRatio(slapTotalVolts(2.f, 1.f)) == doctest::Approx(4.0f));
}

// --- OrchHits (unisonIndex=2) ---

TEST_CASE("OrchHits: Unison(2), 0V -> 1.0x") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(2.f, 0.f)) == doctest::Approx(1.0f));
}

TEST_CASE("OrchHits: Unison(2), +1V -> 2.0x") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(2.f, 1.f)) == doctest::Approx(2.0f));
}

TEST_CASE("OrchHits: Unison(2), -1V -> 0.5x") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(2.f, -1.f)) == doctest::Approx(0.5f));
}

TEST_CASE("OrchHits: Unison(2), 1/12 V -> one semitone up") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(2.f, 1.f / 12.f)) ==
          doctest::Approx(std::pow(2.f, 1.f / 12.f)));
}

TEST_CASE("OrchHits: -2oct(0), 0V -> 0.25x") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(0.f, 0.f)) == doctest::Approx(0.25f));
}

TEST_CASE("OrchHits: +2oct(4), 0V -> 4.0x") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(4.f, 0.f)) == doctest::Approx(4.0f));
}

TEST_CASE("OrchHits: octave transpose stacks on top of pitchCV") {
    CHECK(calc1VperOctPitchRatio(orchTotalVolts(3.f, 1.f)) == doctest::Approx(4.0f));
}
