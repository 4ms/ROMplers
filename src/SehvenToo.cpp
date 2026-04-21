#include "SehvenTooSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SehvenToo : DrumMachineBaseModule<SehvenToo> {
	static constexpr auto drums = std::array{
		NamedSample{"Conga Lo", STCongaL, 2.f},
		NamedSample{"Conga Hi", STCongaHOpen, 2.f},
		NamedSample{"Conga Hi Mute", STCongaHMute, 2.f},
		NamedSample{"Bongo Lo", STBongoL, 2.f},
		NamedSample{"Bongo Hi", STBongoH, 2.f},
		NamedSample{"Timbale Lo", STTimbaleL},
		NamedSample{"Timbale Hi", STTimbaleH},
		NamedSample{"Agogo Lo", STAgogoL, 2.f},
		NamedSample{"Agogo Hi", STAgogoH, 2.f},
		NamedSample{"Maraca", STMaracas, 2.f},
		NamedSample{"Cabasa", STCabasa, 2.f},
		NamedSample{"Whistle", STWhistle, 2.f},
	};
};

struct SehvenTooWidget : DrumMachine12BaseWidget<SehvenTooWidget, SehvenToo> {
	static constexpr auto panel = "res/panels/SehvenToo.svg";
	using DrumMachine12BaseWidget::DrumMachine12BaseWidget;
};

Model *modelSehvenToo = createModel<SehvenToo, SehvenTooWidget>("SehvenToo");
