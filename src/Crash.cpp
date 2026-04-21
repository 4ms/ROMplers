#include "CrashSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Crash : OneShotBaseModule<Crash> {
	static constexpr std::array samples = {
		Sample{Crash1},
		Sample{Crash2},
		Sample{Crash3},
		Sample{Crash4},
		Sample{Crash5},
		Sample{Crash6},
		Sample{Crash7},
		Sample{Crash8},
		Sample{Crash9},
		Sample{Crash10},
		Sample{Crash11},
		Sample{Crash12},
		Sample{Crash13},
		Sample{Crash14},
		Sample{Crash15},
		Sample{Crash16},
	};
};

struct CrashWidget : OneShotBaseWidget<CrashWidget, Crash> {
	static constexpr auto panel = "res/panels/Crash.svg";
	using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelCrash = createModel<Crash, CrashWidget>("Crash");
