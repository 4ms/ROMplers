#include "PercussionSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Percussion : OneShotBaseModule<Percussion> {
	static constexpr std::array samples = {
		Sample{Percussion1},
		Sample{Percussion2},
		Sample{Percussion3},
		Sample{Percussion4},
		Sample{Percussion5},
		Sample{Percussion6},
		Sample{Percussion7},
		Sample{Percussion8},
		Sample{Percussion9},
		Sample{Percussion10},
		Sample{Percussion11},
		Sample{Percussion12},
		Sample{Percussion13},
		Sample{Percussion14},
		Sample{Percussion15},
		Sample{Percussion16},
	};
};

struct PercussionWidget : OneShotBaseWidget<PercussionWidget, Percussion> {
	static constexpr auto panel = "res/panels/Percussion.svg";
	using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelPercussion = createModel<Percussion, PercussionWidget>("Percussion");
