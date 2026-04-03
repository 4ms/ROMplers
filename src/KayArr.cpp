#include "KayArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct KayArr : DrumMachineBaseModule<KayArr> {
  static constexpr float outputScale = 2.f;
  static constexpr auto drums = std::array{
      NamedSample{"Kick", KRKick},
      NamedSample{"Snare", KRSnare},
      NamedSample{"Tom", KRTom},
      NamedSample{"Closed Hat", KRClosedHat},
      NamedSample{"Open Hat", KROpenHat},
      NamedSample{"Clave", KRClave},
      NamedSample{"Rimshot", KRRimshot},
      NamedSample{"Cowbell", KRCowbell},
      NamedSample{"Cymbal", KRCymbal},
      NamedSample{"Conga", KRConga},
  };
};

struct KayArrWidget : DrumMachine10BaseWidget<KayArrWidget, KayArr> {
  static constexpr auto panel = "res/panels/KayArr.svg";
  using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");
