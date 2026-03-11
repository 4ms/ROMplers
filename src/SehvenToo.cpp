#include "SehvenTooSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SehvenToo : DrumMachineBaseModule<SehvenToo> {
  static constexpr auto drums = std::array{
      NamedSample{"Conga Lo", STCongaL},
      NamedSample{"Conga Hi", STCongaHOpen},
      NamedSample{"Conga Hi Mute", STCongaHMute},
      NamedSample{"Bongo Lo", STBongoL},
      NamedSample{"Bongo Hi", STBongoH},
      NamedSample{"Timbale Lo", STTimbaleL},
      NamedSample{"Timbale Hi", STTimbaleH},
      NamedSample{"Agogo Lo", STAgogoL},
      NamedSample{"Agogo Hi", STAgogoH},
      NamedSample{"Maraca", STMaracas},
      NamedSample{"Cabasa", STCabasa},
      NamedSample{"Whistle", STWhistle},

  };
};

struct SehvenTooWidget : DrumMachine12BaseWidget<SehvenTooWidget, SehvenToo> {
  static constexpr auto panel = "res/panels/SehvenToo.svg";
  using DrumMachine12BaseWidget::DrumMachine12BaseWidget;
};

Model *modelSehvenToo = createModel<SehvenToo, SehvenTooWidget>("SehvenToo");
