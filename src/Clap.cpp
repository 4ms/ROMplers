#include "ClapSamples.hpp"
#include "one_shot_base.hh"
#include "sample.hh"

struct Clap : OneShotBaseModule<Clap> {

	static constexpr std::array samples = {
		Sample{Clap1},
		Sample{Clap2},
		Sample{Clap3},
		Sample{Clap4},
		Sample{Clap5},
		Sample{Clap6},
		Sample{Clap7},
		Sample{Clap8},
		Sample{Clap9},
		Sample{Clap10},
		Sample{Clap11},
		Sample{Clap12},
		Sample{Clap13},
		Sample{Clap14},
		Sample{Clap15},
		Sample{Clap16},
	};
};

struct ClapWidget : OneShotBaseWidget<ClapWidget, Clap> {
	static constexpr auto panel = "res/panels/Clap.svg";
	using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelClap = createModel<Clap, ClapWidget>("Clap");
