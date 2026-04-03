#include "SeaArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SeaArr : DrumMachineBaseModule<SeaArr> {
  static constexpr float outputScale = 2.f;
  static constexpr float sumScale = 4.f / 3.f;
  static constexpr auto drums = std::array{
      NamedSample{"Kick", SAKick},       NamedSample{"Snare", SASnare},
      NamedSample{"Hat", SAHiHat},       NamedSample{"Hat Metal", SAHiHatMetal},
      NamedSample{"Rimshot", SARim},     NamedSample{"Cowbell", SACowbell},
      NamedSample{"Conga", SACongaL},    NamedSample{"Bongo Lo", SABongoL},
      NamedSample{"Bongo Hi", SABongoH}, NamedSample{"Tambourine", SATamb},
      NamedSample{"Guiro", SAGuiro},     NamedSample{"Cymbal", SACym},

  };
};

struct SeaArrWidget : DrumMachine12BaseWidget<SeaArrWidget, SeaArr> {
  static constexpr auto panel = "res/panels/SeaArr.svg";
  using DrumMachine12BaseWidget::DrumMachine12BaseWidget;
};

Model *modelSeaArr = createModel<SeaArr, SeaArrWidget>("SeaArr");
