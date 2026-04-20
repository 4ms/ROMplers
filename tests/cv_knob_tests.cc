#include "../src/cv_knob_utils.hh"
#include <doctest/doctest.h>

TEST_CASE("CV and knob combine correctly") {
	// CV = 0: knob value is used directly
	CHECK(indexed_cv_knob<8>(0, 0) == 0);
	CHECK(indexed_cv_knob<8>(0, 1) == 1);
	CHECK(indexed_cv_knob<8>(0, 7) == 7);

	// Clamp at max/min
	CHECK(indexed_cv_knob<8>(0, -1) == 0);
	CHECK(indexed_cv_knob<8>(0, -2) == 0);
	CHECK(indexed_cv_knob<8>(0, 8) == 7);
	CHECK(indexed_cv_knob<8>(0, 9) == 7);

	// Knob = 0: cv value is used scaled by 5V / range
	constexpr float cv_step = 5.f / 8;
	CHECK(indexed_cv_knob<8>(cv_step * 1, 0) == 1);
	CHECK(indexed_cv_knob<8>(cv_step * 2, 0) == 2);
	CHECK(indexed_cv_knob<8>(cv_step * 3, 0) == 3);
	CHECK(indexed_cv_knob<8>(cv_step * 4, 0) == 4);
	CHECK(indexed_cv_knob<8>(cv_step * 5, 0) == 5);
	CHECK(indexed_cv_knob<8>(cv_step * 6, 0) == 6);
	CHECK(indexed_cv_knob<8>(cv_step * 7, 0) == 7);

	// Clamp at min/max
	CHECK(indexed_cv_knob<8>(-100, 0) == 0);
	CHECK(indexed_cv_knob<8>(-10, 0) == 0);
	CHECK(indexed_cv_knob<8>(-1, 0) == 0);
	CHECK(indexed_cv_knob<8>(cv_step * -2, 0) == 0);
	CHECK(indexed_cv_knob<8>(cv_step * -1, 0) == 0);
	CHECK(indexed_cv_knob<8>(cv_step * 8, 0) == 7);
	CHECK(indexed_cv_knob<8>(cv_step * 9, 0) == 7);
	CHECK(indexed_cv_knob<8>(10, 0) == 7);
	CHECK(indexed_cv_knob<8>(100, 0) == 7);

	// Knob at min: CV 0V to +5V should go whole range of indexes
	CHECK(indexed_cv_knob<8>(0.f, 0) == 0);
	CHECK(indexed_cv_knob<8>(5.f, 0) == 7);

	// Knob at max: CV 0V to -5V should go whole range of indexes
	CHECK(indexed_cv_knob<8>(0.f, 7) == 7);
	CHECK(indexed_cv_knob<8>(cv_step * -1, 7) == 6);
	CHECK(indexed_cv_knob<8>(cv_step * -2, 7) == 5);
	CHECK(indexed_cv_knob<8>(cv_step * -3, 7) == 4);
	CHECK(indexed_cv_knob<8>(cv_step * -4, 7) == 3);
	CHECK(indexed_cv_knob<8>(cv_step * -5, 7) == 2);
	CHECK(indexed_cv_knob<8>(cv_step * -6, 7) == 1);
	CHECK(indexed_cv_knob<8>(cv_step * -7, 7) == 0);
	CHECK(indexed_cv_knob<8>(-5.f, 7) == 0);
}
