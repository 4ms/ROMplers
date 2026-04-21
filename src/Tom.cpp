#include "TomSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Tom : OneShotBaseModule<Tom> {
	static constexpr std::array samples = {
		Sample{Tom1},
		Sample{Tom2},
		Sample{Tom3},
		Sample{Tom4},
		Sample{Tom5},
		Sample{Tom6},
		Sample{Tom7},
		Sample{Tom8},
		Sample{Tom9},
		Sample{Tom10},
		Sample{Tom11},
		Sample{Tom12},
		Sample{Tom13},
		Sample{Tom14},
		Sample{Tom15},
		Sample{Tom16},
	};
};

struct TomWidget : OneShotBaseWidget<TomWidget, Tom> {
	static constexpr auto panel = "res/panels/Tom.svg";
	using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelTom = createModel<Tom, TomWidget>("Tom");
