#include "KayArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct KayArr : DrumMachineBaseModule<KayArr> {
  static constexpr auto drums = std::array{
      DrumSample{"Kick", KRKick},        DrumSample{"Snare", KRSnare},
      DrumSample{"Tom", KRTom},          DrumSample{"Closed Hat", KRClosedHat},
      DrumSample{"Open Hat", KROpenHat}, DrumSample{"Clave", KRClave},
      DrumSample{"Rimshot", KRRimshot},  DrumSample{"Cowbell", KRCowbell},
      DrumSample{"Cymbal", KRCymbal},    DrumSample{"Conga", KRConga},
  };
};

struct KayArrWidget : DrumMachine10BaseWidget<KayArrWidget, KayArr> {
  static constexpr auto panel = "res/panels/KayArr.svg";
  using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");
