#include "RideSamples.hpp"
#include "one_shot_base.hh"
#include "plugin.hpp"
#include "sample.hh"

struct Ride : OneShotBaseModule<Ride> {
  static constexpr auto min_rate = .01f;
  static constexpr auto max_rate = 2.f;
  static constexpr std::array samples = {
      Sample{Ride1},  Sample{Ride2},  Sample{Ride3},  Sample{Ride4},
      Sample{Ride5},  Sample{Ride6},  Sample{Ride7},  Sample{Ride8},
      Sample{Ride9},  Sample{Ride10}, Sample{Ride11}, Sample{Ride12},
      Sample{Ride13}, Sample{Ride14}, Sample{Ride15}, Sample{Ride16},
  };
};

struct RideWidget : OneShotBaseWidget<RideWidget, Ride> {
  static constexpr auto panel = "res/panels/Ride.svg";
  using OneShotBaseWidget::OneShotBaseWidget;
};

Model *modelRide = createModel<Ride, RideWidget>("Ride");
