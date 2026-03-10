#include "RimshotSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Rimshot : OneShotBaseModule<Rimshot> {
  static constexpr auto min_rate = .01f;
  static constexpr auto max_rate = 2.f;
  static constexpr std::array samples = {
      Sample{Rimshot1},  Sample{Rimshot2},  Sample{Rimshot3},
      Sample{Rimshot4},  Sample{Rimshot5},  Sample{Rimshot6},
      Sample{Rimshot7},  Sample{Rimshot8},  Sample{Rimshot9},
      Sample{Rimshot10}, Sample{Rimshot11}, Sample{Rimshot12},
      Sample{Rimshot13}, Sample{Rimshot14}, Sample{Rimshot15},
      Sample{Rimshot16},
  };
};

struct RimshotWidget : OneShotBaseWidget<RimshotWidget, Rimshot> {
  static constexpr auto panel = "res/panels/Rimshot.svg";
  using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelRimshot = createModel<Rimshot, RimshotWidget>("Rimshot");
