#include <doctest/doctest.h>
#include "dsp_utils.hh"

TEST_CASE("calcDecayMod: knob min, no CV -> 0.0") {
    CHECK(calcDecayMod(0.f, 0.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayMod: knob max, no CV -> 1.0") {
    CHECK(calcDecayMod(1.f, 0.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayMod: knob center, no CV -> 0.5") {
    CHECK(calcDecayMod(0.5f, 0.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayMod: knob 0.25, no CV -> 0.25") {
    CHECK(calcDecayMod(0.25f, 0.f) == doctest::Approx(0.25f));
}

TEST_CASE("calcDecayMod: knob 0.75, no CV -> 0.75") {
    CHECK(calcDecayMod(0.75f, 0.f) == doctest::Approx(0.75f));
}

TEST_CASE("calcDecayMod: knob min, +5V CV -> 0.5 (CV lifts from floor)") {
    CHECK(calcDecayMod(0.f, 5.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayMod: knob max, -5V CV -> 0.5 (CV pulls from ceiling)") {
    CHECK(calcDecayMod(1.f, -5.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayMod: knob center, +5V CV -> 1.0") {
    CHECK(calcDecayMod(0.5f, 5.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayMod: knob center, -5V CV -> 0.0") {
    CHECK(calcDecayMod(0.5f, -5.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayMod: knob 0.25, +2.5V CV -> 0.5") {
    CHECK(calcDecayMod(0.25f, 2.5f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayMod: knob min, -5V CV -> 0.0 (clamps at floor)") {
    CHECK(calcDecayMod(0.f, -5.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayMod: knob max, +5V CV -> 1.0 (clamps at ceiling)") {
    CHECK(calcDecayMod(1.f, 5.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayMod: over-range positive CV clamps to 1.0") {
    CHECK(calcDecayMod(0.f, 10.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayMod: over-range negative CV clamps to 0.0") {
    CHECK(calcDecayMod(1.f, -10.f) == doctest::Approx(0.f));
}

// calcDecayModScaled: knob in 0..5 range, CV in ±10V (scaled internally by 0.5)
// Used by OrchHits (knob 0.01..5 directly) and Slap (knob 0..1, caller multiplies by 5)

TEST_CASE("calcDecayModScaled: knob 0, no CV -> 0.0") {
    CHECK(calcDecayModScaled(0.f, 0.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayModScaled: knob 5, no CV -> 1.0") {
    CHECK(calcDecayModScaled(5.f, 0.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayModScaled: knob 2.5, no CV -> 0.5") {
    CHECK(calcDecayModScaled(2.5f, 0.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayModScaled: knob 1.25, no CV -> 0.25") {
    CHECK(calcDecayModScaled(1.25f, 0.f) == doctest::Approx(0.25f));
}

TEST_CASE("calcDecayModScaled: knob 3.75, no CV -> 0.75") {
    CHECK(calcDecayModScaled(3.75f, 0.f) == doctest::Approx(0.75f));
}

TEST_CASE("calcDecayModScaled: knob 0, +10V CV -> 1.0") {
    CHECK(calcDecayModScaled(0.f, 10.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayModScaled: knob 5, -10V CV -> 0.0") {
    CHECK(calcDecayModScaled(5.f, -10.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayModScaled: knob 0, +5V CV -> 0.5 (CV scaled by 0.5 internally)") {
    CHECK(calcDecayModScaled(0.f, 5.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayModScaled: knob 5, -5V CV -> 0.5") {
    CHECK(calcDecayModScaled(5.f, -5.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcDecayModScaled: knob 2.5, +10V CV clamps to 1.0") {
    CHECK(calcDecayModScaled(2.5f, 10.f) == doctest::Approx(1.f));
}

TEST_CASE("calcDecayModScaled: knob 2.5, -10V CV clamps to 0.0") {
    CHECK(calcDecayModScaled(2.5f, -10.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayModScaled: knob 0, -10V CV clamps to 0.0") {
    CHECK(calcDecayModScaled(0.f, -10.f) == doctest::Approx(0.f));
}

TEST_CASE("calcDecayModScaled: knob 5, +10V CV clamps to 1.0") {
    CHECK(calcDecayModScaled(5.f, 10.f) == doctest::Approx(1.f));
}

// calcLengthMod: KayOne-style length — knob (0..1) + CV * 0.1, hard floor at 0.1
// Returns length ratio 0.1..1.0 directly

TEST_CASE("calcLengthMod: knob min, no CV -> 0.1 (hard floor)") {
    CHECK(calcLengthMod(0.f, 0.f) == doctest::Approx(0.1f));
}

TEST_CASE("calcLengthMod: knob max, no CV -> 1.0") {
    CHECK(calcLengthMod(1.f, 0.f) == doctest::Approx(1.f));
}

TEST_CASE("calcLengthMod: knob 0.5, no CV -> 0.5") {
    CHECK(calcLengthMod(0.5f, 0.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcLengthMod: knob 0.25, no CV -> 0.25") {
    CHECK(calcLengthMod(0.25f, 0.f) == doctest::Approx(0.25f));
}

TEST_CASE("calcLengthMod: knob 0.75, no CV -> 0.75") {
    CHECK(calcLengthMod(0.75f, 0.f) == doctest::Approx(0.75f));
}

TEST_CASE("calcLengthMod: knob 0, +5V CV -> 0.5") {
    CHECK(calcLengthMod(0.f, 5.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcLengthMod: knob 1, -5V CV -> 0.5") {
    CHECK(calcLengthMod(1.f, -5.f) == doctest::Approx(0.5f));
}

TEST_CASE("calcLengthMod: knob 0, -10V CV clamps to 0.1 (floor)") {
    CHECK(calcLengthMod(0.f, -10.f) == doctest::Approx(0.1f));
}

TEST_CASE("calcLengthMod: knob 1, +10V CV clamps to 1.0 (ceiling)") {
    CHECK(calcLengthMod(1.f, 10.f) == doctest::Approx(1.f));
}

TEST_CASE("calcLengthMod: knob 0, +1V CV -> 0.1 (exactly at floor)") {
    CHECK(calcLengthMod(0.f, 1.f) == doctest::Approx(0.1f));
}

TEST_CASE("calcLengthMod: knob 0.5, +5V CV -> 1.0") {
    CHECK(calcLengthMod(0.5f, 5.f) == doctest::Approx(1.f));
}

TEST_CASE("calcLengthMod: knob 0.5, -5V CV clamps to 0.1 (floor)") {
    CHECK(calcLengthMod(0.5f, -5.f) == doctest::Approx(0.1f));
}
