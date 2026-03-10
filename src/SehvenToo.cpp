#include "SehvenTooSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SehvenToo : DrumMachineBaseModule<SehvenToo> {
  static constexpr auto drums = std::array{
      DrumSample{"Conga Lo", STCongaL},
      DrumSample{"Conga Hi", STCongaHOpen},
      DrumSample{"Conga Hi Mute", STCongaHMute},
      DrumSample{"Bongo Lo", STBongoL},
      DrumSample{"Bongo Hi", STBongoH},
      DrumSample{"Timbale Lo", STTimbaleL},
      DrumSample{"Timbale Hi", STTimbaleH},
      DrumSample{"Agogo Lo", STAgogoL},
      DrumSample{"Agogo Hi", STAgogoH},
      DrumSample{"Maraca", STMaracas},
      DrumSample{"Cabasa", STCabasa},
      DrumSample{"Whistle", STWhistle},

  };
};

struct SehvenTooWidget : DrumMachine12BaseWidget<SehvenTooWidget, SehvenToo> {
  static constexpr auto panel = "res/panels/SehvenToo.svg";
  using DrumMachine12BaseWidget::DrumMachine12BaseWidget;
};

Model *modelSehvenToo = createModel<SehvenToo, SehvenTooWidget>("SehvenToo");
