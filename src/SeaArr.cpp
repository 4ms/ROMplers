#include "SeaArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SeaArr : DrumMachineBaseModule<SeaArr> {
  static constexpr auto drums = std::array{
      DrumSample{"Kick", SAKick},       DrumSample{"Snare", SASnare},
      DrumSample{"Hat", SAHiHat},       DrumSample{"Hat Metal", SAHiHatMetal},
      DrumSample{"Rimshot", SARim},     DrumSample{"Cowbell", SACowbell},
      DrumSample{"Conga", SACongaL},    DrumSample{"Bongo Lo", SABongoL},
      DrumSample{"Bongo Hi", SABongoH}, DrumSample{"Tambourine", SATamb},
      DrumSample{"Guiro", SAGuiro},     DrumSample{"Cymbal", SACym},

  };
};

struct SeaArrWidget : DrumMachine12BaseWidget<SeaArrWidget, SeaArr> {
  static constexpr auto panel = "res/panels/SeaArr.svg";
  using DrumMachine12BaseWidget::DrumMachine12BaseWidget;
};

Model *modelSeaArr = createModel<SeaArr, SeaArrWidget>("SeaArr");
