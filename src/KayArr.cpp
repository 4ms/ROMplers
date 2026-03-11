#include "KayArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct KayArr : DrumMachineBaseModule<KayArr> {
  static constexpr auto drums = std::array{
      DrumSample{"Kick Trig", KRKick},
      DrumSample{"Snare Trig", KRSnare},
      DrumSample{"Tom Trig", KRTom},
      DrumSample{"Closed Hat Trig", KRClosedHat},
      DrumSample{"Open Hat Trig", KROpenHat},
      DrumSample{"Clave Trig", KRClave},
      DrumSample{"Rimshot Trig", KRRimshot},
      DrumSample{"Cowbell Trig", KRCowbell},
      DrumSample{"Cymbal Trig", KRCymbal},
      DrumSample{"Conga Trig", KRConga},
  };
};

struct KayArrWidget : DrumMachine10BaseWidget<KayArrWidget, KayArr> {
  static constexpr auto panel = "res/panels/KayArr.svg";
  using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");
