#include "KayArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct KayArr : DrumMachineBaseModule<KayArr> {
  static constexpr auto drums = std::array{
      NamedSample{"Kick", KRKick, 2.f},
      NamedSample{"Snare", KRSnare, 2.f},
      NamedSample{"Closed Hat", KRClosedHat, 2.f},
      NamedSample{"Open Hat", KROpenHat, 2.f},
      NamedSample{"Low Tom", KRTom, 2.f},
      NamedSample{"Conga", KRConga, 2.f},
      NamedSample{"Clave", KRClave, 2.f},
      NamedSample{"Rimshot", KRRimshot, 2.f},
      NamedSample{"Cowbell", KRCowbell, 2.f},
      NamedSample{"Cymbal", KRCymbal, 2.f},
  };
};

struct KayArrWidget : DrumMachine10BaseWidget<KayArrWidget, KayArr> {
  static constexpr auto panel = "res/panels/KayArr.svg";
  using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");
