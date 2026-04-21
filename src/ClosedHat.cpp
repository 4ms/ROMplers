#include "ClosedHatSamples.hpp"
#include "one_shot_base.hh"
#include "sample.hh"

struct ClosedHat : OneShotBaseModule<ClosedHat> {
	static constexpr std::array samples = {
		Sample{ClosedHat1},
		Sample{ClosedHat2},
		Sample{ClosedHat3},
		Sample{ClosedHat4},
		Sample{ClosedHat5},
		Sample{ClosedHat6},
		Sample{ClosedHat7},
		Sample{ClosedHat8},
		Sample{ClosedHat9},
		Sample{ClosedHat10},
		Sample{ClosedHat11},
		Sample{ClosedHat12},
		Sample{ClosedHat13},
		Sample{ClosedHat14},
		Sample{ClosedHat15},
		Sample{ClosedHat16},
	};
};

struct ClosedHatWidget : OneShotBaseWidget<ClosedHatWidget, ClosedHat> {
	static constexpr auto panel = "res/panels/ClosedHat.svg";
	using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelClosedHat = createModel<ClosedHat, ClosedHatWidget>("ClosedHat");
