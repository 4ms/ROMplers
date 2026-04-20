#include <doctest/doctest.h>
#include "dsp_utils.hh"

// calcPitchRatio: normalizedPitch 0..1 -> playback ratio
// 0.0 -> 0.01x, 0.5 -> 1.0x (unity), 1.0 -> 2.0x

TEST_CASE("calcPitchRatio: min position -> 0.01x") {
    CHECK(calcPitchRatio(0.f) == doctest::Approx(0.01f));
}

TEST_CASE("calcPitchRatio: center position -> 1.0x (unity)") {
    CHECK(calcPitchRatio(0.5f) == doctest::Approx(1.0f));
}

TEST_CASE("calcPitchRatio: max position -> 2.0x") {
    CHECK(calcPitchRatio(1.f) == doctest::Approx(2.0f));
}

TEST_CASE("calcPitchRatio: 25% -> midpoint of lower half (0.505x)") {
    CHECK(calcPitchRatio(0.25f) == doctest::Approx(0.505f));
}

TEST_CASE("calcPitchRatio: 75% -> midpoint of upper half (1.5x)") {
    CHECK(calcPitchRatio(0.75f) == doctest::Approx(1.5f));
}

TEST_CASE("calcPitchRatio: 10% -> linear in lower half") {
    CHECK(calcPitchRatio(0.1f) == doctest::Approx(0.01f + 0.1f * 1.98f));
}

TEST_CASE("calcPitchRatio: 90% -> linear in upper half") {
    CHECK(calcPitchRatio(0.9f) == doctest::Approx(1.0f + 0.4f * 2.0f));
}

TEST_CASE("calcPitchRatio: boundary is continuous at 0.5 from below") {
    CHECK(calcPitchRatio(0.5f - 1e-6f) == doctest::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("calcPitchRatio: boundary is continuous at 0.5 from above") {
    CHECK(calcPitchRatio(0.5f + 1e-6f) == doctest::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("calcPitchRatio: lower half is strictly increasing") {
    CHECK(calcPitchRatio(0.1f) < calcPitchRatio(0.2f));
    CHECK(calcPitchRatio(0.2f) < calcPitchRatio(0.3f));
    CHECK(calcPitchRatio(0.3f) < calcPitchRatio(0.4f));
}

TEST_CASE("calcPitchRatio: upper half is strictly increasing") {
    CHECK(calcPitchRatio(0.6f) < calcPitchRatio(0.7f));
    CHECK(calcPitchRatio(0.7f) < calcPitchRatio(0.8f));
    CHECK(calcPitchRatio(0.8f) < calcPitchRatio(0.9f));
}
