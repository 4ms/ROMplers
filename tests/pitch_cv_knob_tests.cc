#include "../src/cv_knob_utils.hh"
#include <doctest/doctest.h>

constexpr float MIN = 0.01f;
constexpr float MAX = 2.0f;

TEST_CASE("pitch cv+knob: cv=0, knob -1 to +1 sweeps full MIN..MAX range") {
	CHECK(pitch_cv_knob(0.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob(0.f, 1.f) == doctest::Approx(MAX));

	// knob = 0 outputs 1.0
	CHECK(pitch_cv_knob(0.f, 0.f) == doctest::Approx(1.f));

	// Linear between -1 and 0
	CHECK(pitch_cv_knob(0.f, -0.5f) == doctest::Approx(MIN + 0.5f * (1.f - MIN)));

	// Linear between 0 and 1
	CHECK(pitch_cv_knob(0.f, 0.5f) == doctest::Approx(1.f + 0.5f * (MAX - 1.f)));
}

TEST_CASE("pitch cv+knob: output clamped to [MIN, MAX]") {
	// Knob beyond +1 with positive CV cannot exceed MAX
	CHECK(pitch_cv_knob(5.f, 1.f) == doctest::Approx(MAX));
	CHECK(pitch_cv_knob(10.f, 1.f) == doctest::Approx(MAX));
	CHECK(pitch_cv_knob(10.f, 0.5f) == doctest::Approx(MAX));

	// Knob beyond -1 with negative CV cannot go below MIN
	CHECK(pitch_cv_knob(-5.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob(-10.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob(-10.f, -0.5f) == doctest::Approx(MIN));
}

TEST_CASE("pitch cv+knob: cv=-5 shifts range to [MIN, midpoint]") {
	// Knob -1 to 0 all clamp to MIN
	CHECK(pitch_cv_knob(-5.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob(-5.f, -0.5f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob(-5.f, 0.f) == doctest::Approx(MIN));

	// Knob 0 to 1 maps linearly from MIN to midpoint
	CHECK(pitch_cv_knob(-5.f, 0.5f) == doctest::Approx(MIN + 0.5f * (1.f - MIN)));
	CHECK(pitch_cv_knob(-5.f, 1.f) == doctest::Approx(1.f));
}

TEST_CASE("pitch cv+knob: cv=+5 shifts range to [midpoint, MAX]") {
	// Knob -1 to 0 maps linearly from midpoint to MAX
	CHECK(pitch_cv_knob(5.f, -1.f) == doctest::Approx(1.f));
	CHECK(pitch_cv_knob(5.f, -0.5f) == doctest::Approx(1.f + 0.5f * (MAX - 1.f)));
	CHECK(pitch_cv_knob(5.f, 0.f) == doctest::Approx(MAX));

	// Knob 0 to 1 all clamp to MAX
	CHECK(pitch_cv_knob(5.f, 0.5f) == doctest::Approx(MAX));
	CHECK(pitch_cv_knob(5.f, 1.f) == doctest::Approx(MAX));
}
