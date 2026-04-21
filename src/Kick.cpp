#include "KickSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Kick : OneShotBaseModule<Kick> {
	static constexpr std::array samples = {
		Sample{Kick1},
		Sample{Kick2},
		Sample{Kick3},
		Sample{Kick4},
		Sample{Kick5},
		Sample{Kick6},
		Sample{Kick7},
		Sample{Kick8},
		Sample{Kick9},
		Sample{Kick10},
		Sample{Kick11},
		Sample{Kick12},
		Sample{Kick13},
		Sample{Kick14},
		Sample{Kick15},
		Sample{Kick16},
	};
};

struct KickWidget : OneShotBaseWidget<KickWidget, Kick> {
	static constexpr auto panel = "res/panels/Kick.svg";
	using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelKick = createModel<Kick, KickWidget>("Kick");
