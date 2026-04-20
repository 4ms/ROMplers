#include "../src/cv_knob_utils.hh"
#include <doctest/doctest.h>

constexpr float MIN = 0.5f;
constexpr float MAX = 2.0f;
constexpr float MID = (MIN + MAX) * 0.5f;

TEST_CASE("pitch cv+knob: cv=0, knob sweeps full MIN..MAX range") {
	CHECK(pitch_cv_knob<MIN, MAX>(0.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob<MIN, MAX>(0.f, 0.f) == doctest::Approx(MID));
	CHECK(pitch_cv_knob<MIN, MAX>(0.f, 1.f) == doctest::Approx(MAX));

	// Linear between -1 and 1
	CHECK(pitch_cv_knob<MIN, MAX>(0.f, -0.5f) == doctest::Approx(MIN + 0.25f * (MAX - MIN)));
	CHECK(pitch_cv_knob<MIN, MAX>(0.f, 0.5f) == doctest::Approx(MIN + 0.75f * (MAX - MIN)));
}

TEST_CASE("pitch cv+knob: output clamped to [MIN, MAX]") {
	// Knob beyond +1 with positive CV cannot exceed MAX
	CHECK(pitch_cv_knob<MIN, MAX>(5.f, 1.f) == doctest::Approx(MAX));
	CHECK(pitch_cv_knob<MIN, MAX>(10.f, 1.f) == doctest::Approx(MAX));
	CHECK(pitch_cv_knob<MIN, MAX>(10.f, 0.5f) == doctest::Approx(MAX));

	// Knob beyond -1 with negative CV cannot go below MIN
	CHECK(pitch_cv_knob<MIN, MAX>(-5.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob<MIN, MAX>(-10.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob<MIN, MAX>(-10.f, -0.5f) == doctest::Approx(MIN));
}

TEST_CASE("pitch cv+knob: cv=-5 shifts range to [MIN, midpoint]") {
	// Knob -1 to 0 all clamp to MIN
	CHECK(pitch_cv_knob<MIN, MAX>(-5.f, -1.f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob<MIN, MAX>(-5.f, -0.5f) == doctest::Approx(MIN));
	CHECK(pitch_cv_knob<MIN, MAX>(-5.f, 0.f) == doctest::Approx(MIN));

	// Knob 0 to 1 maps linearly from MIN to midpoint
	CHECK(pitch_cv_knob<MIN, MAX>(-5.f, 0.5f) == doctest::Approx(MIN + 0.25f * (MAX - MIN)));
	CHECK(pitch_cv_knob<MIN, MAX>(-5.f, 1.f) == doctest::Approx(MID));
}

TEST_CASE("pitch cv+knob: cv=+5 shifts range to [midpoint, MAX]") {
	// Knob -1 to 0 maps linearly from midpoint to MAX
	CHECK(pitch_cv_knob<MIN, MAX>(5.f, -1.f) == doctest::Approx(MID));
	CHECK(pitch_cv_knob<MIN, MAX>(5.f, -0.5f) == doctest::Approx(MIN + 0.75f * (MAX - MIN)));
	CHECK(pitch_cv_knob<MIN, MAX>(5.f, 0.f) == doctest::Approx(MAX));

	// Knob 0 to 1 all clamp to MAX
	CHECK(pitch_cv_knob<MIN, MAX>(5.f, 0.5f) == doctest::Approx(MAX));
	CHECK(pitch_cv_knob<MIN, MAX>(5.f, 1.f) == doctest::Approx(MAX));
}
