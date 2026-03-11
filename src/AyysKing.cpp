#include "AyysKingSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct AyysKing : DrumMachineBaseModule<AyysKing> {
  static constexpr auto drums = std::array{
      NamedSample{"Kick", AKKick},
      NamedSample{"Snare 1", AKSnare},
      NamedSample{"Snare 2", AKSnare2},
      NamedSample{"Closed Hat", AKClosedHiHat},
      NamedSample{"Open Hat", AKOpenHiHat},
      NamedSample{"Bongo 1", AKBongo},
      NamedSample{"Bongo 2", AKBongo2},
      NamedSample{"Bongo 3", AKBongo3},
      NamedSample{"Clave", AKWood},
      NamedSample{"Cymbal", AKCrash},
  };
};

struct AyysKingWidget : DrumMachine10BaseWidget<AyysKingWidget, AyysKing> {
  static constexpr auto panel = "res/panels/AyysKing.svg";
  using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelAyysKing = createModel<AyysKing, AyysKingWidget>("AyysKing");
