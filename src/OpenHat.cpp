#include "OpenHatSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct OpenHat : OneShotBaseModule<OpenHat> {
  static constexpr auto min_rate = 2.f;
  static constexpr auto max_rate = 8.f;

  static constexpr std::array samples = {
      Sample{OpenHat1},  Sample{OpenHat2},  Sample{OpenHat3},
      Sample{OpenHat4},  Sample{OpenHat5},  Sample{OpenHat6},
      Sample{OpenHat7},  Sample{OpenHat8},  Sample{OpenHat9},
      Sample{OpenHat10}, Sample{OpenHat11}, Sample{OpenHat12},
      Sample{OpenHat13}, Sample{OpenHat14}, Sample{OpenHat15},
      Sample{OpenHat16},
  };
};

struct OpenHatWidget : OneShotBaseWidget<OpenHatWidget, OpenHat> {
  static constexpr auto panel = "res/panels/OpenHat.svg";
  using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelOpenHat = createModel<OpenHat, OpenHatWidget>("OpenHat");
