#include "SnareSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Snare : OneShotBaseModule<Snare> {
  static constexpr auto min_rate = .01f;
  static constexpr auto max_rate = 2.f;
  static constexpr std::array samples = {
      Sample{Snare1},  Sample{Snare2},  Sample{Snare3},  Sample{Snare4},
      Sample{Snare5},  Sample{Snare6},  Sample{Snare7},  Sample{Snare8},
      Sample{Snare9},  Sample{Snare10}, Sample{Snare11}, Sample{Snare12},
      Sample{Snare13}, Sample{Snare14}, Sample{Snare15}, Sample{Snare16},
  };
};

struct SnareWidget : OneShotBaseWidget<SnareWidget, Snare> {
  static constexpr auto panel = "res/panels/Snare.svg";
  using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelSnare = createModel<Snare, SnareWidget>("Snare");
