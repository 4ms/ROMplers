#include "cv_knob_utils.hh"
#include <doctest/doctest.h>

// piecewiseLinearPitchRatio: normalizedPitch 0..1 -> playback ratio
// 0.0 -> 0.01x, 0.5 -> 1.0x (unity), 1.0 -> 2.0x

TEST_CASE("piecewiseLinearPitchRatio: min position -> 0.01x") {
	CHECK(piecewiseLinearPitchRatio(0.f) == doctest::Approx(0.01f));
}

TEST_CASE("piecewiseLinearPitchRatio: center position -> 1.0x (unity)") {
	CHECK(piecewiseLinearPitchRatio(0.5f) == doctest::Approx(1.0f));
}

TEST_CASE("piecewiseLinearPitchRatio: max position -> 2.0x") {
	CHECK(piecewiseLinearPitchRatio(1.f) == doctest::Approx(2.0f));
}

TEST_CASE("piecewiseLinearPitchRatio: 25% -> midpoint of lower half (0.505x)") {
	CHECK(piecewiseLinearPitchRatio(0.25f) == doctest::Approx(0.505f));
}

TEST_CASE("piecewiseLinearPitchRatio: 75% -> midpoint of upper half (1.5x)") {
	CHECK(piecewiseLinearPitchRatio(0.75f) == doctest::Approx(1.5f));
}

TEST_CASE("piecewiseLinearPitchRatio: 10% -> linear in lower half") {
	CHECK(piecewiseLinearPitchRatio(0.1f) == doctest::Approx(0.01f + 0.1f * 1.98f));
}

TEST_CASE("piecewiseLinearPitchRatio: 90% -> linear in upper half") {
	CHECK(piecewiseLinearPitchRatio(0.9f) == doctest::Approx(1.0f + 0.4f * 2.0f));
}

TEST_CASE("piecewiseLinearPitchRatio: boundary is continuous at 0.5 from below") {
	CHECK(piecewiseLinearPitchRatio(0.5f - 1e-6f) == doctest::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("piecewiseLinearPitchRatio: boundary is continuous at 0.5 from above") {
	CHECK(piecewiseLinearPitchRatio(0.5f + 1e-6f) == doctest::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("piecewiseLinearPitchRatio: lower half is strictly increasing") {
	CHECK(piecewiseLinearPitchRatio(0.1f) < piecewiseLinearPitchRatio(0.2f));
	CHECK(piecewiseLinearPitchRatio(0.2f) < piecewiseLinearPitchRatio(0.3f));
	CHECK(piecewiseLinearPitchRatio(0.3f) < piecewiseLinearPitchRatio(0.4f));
}

TEST_CASE("piecewiseLinearPitchRatio: upper half is strictly increasing") {
	CHECK(piecewiseLinearPitchRatio(0.6f) < piecewiseLinearPitchRatio(0.7f));
	CHECK(piecewiseLinearPitchRatio(0.7f) < piecewiseLinearPitchRatio(0.8f));
	CHECK(piecewiseLinearPitchRatio(0.8f) < piecewiseLinearPitchRatio(0.9f));
}
