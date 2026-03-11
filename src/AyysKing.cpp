#include "AyysKingSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct AyysKing : DrumMachineBaseModule<AyysKing> {
  static constexpr auto drums = std::array{
      DrumSample{"Kick", AKKick},
      DrumSample{"Snare 1", AKSnare},
      DrumSample{"Snare 2", AKSnare2},
      DrumSample{"Closed Hat", AKClosedHiHat},
      DrumSample{"Open Hat", AKOpenHiHat},
      DrumSample{"Bongo 1", AKBongo},
      DrumSample{"Bongo 2", AKBongo2},
      DrumSample{"Bongo 3", AKBongo3},
      DrumSample{"Clave", AKWood},
      DrumSample{"Cymbal", AKCrash},
  };
};

struct AyysKingWidget : DrumMachine10BaseWidget<AyysKingWidget, AyysKing> {
  static constexpr auto panel = "res/panels/AyysKing.svg";
  using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelAyysKing = createModel<AyysKing, AyysKingWidget>("AyysKing");
