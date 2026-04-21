#include "AyysKingSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct AyysKing : DrumMachineBaseModule<AyysKing> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", AKKick, 2.f},
		NamedSample{"Snare 1", AKSnare, 2.f},
		NamedSample{"Snare 2", AKSnare2, 2.f},
		NamedSample{"Closed Hat", AKClosedHiHat, 2.f},
		NamedSample{"Open Hat", AKOpenHiHat, 2.f},
		NamedSample{"Bongo 1", AKBongo, 2.f},
		NamedSample{"Bongo 2", AKBongo2, 2.f},
		NamedSample{"Bongo 3", AKBongo3, 2.f},
		NamedSample{"Clave", AKWood, 2.f},
		NamedSample{"Cymbal", AKCrash, 2.f},
	};
};

struct AyysKingWidget : DrumMachine10BaseWidget<AyysKingWidget, AyysKing> {
	static constexpr auto panel = "res/panels/AyysKing.svg";
	using DrumMachine10BaseWidget::DrumMachine10BaseWidget;
};

Model *modelAyysKing = createModel<AyysKing, AyysKingWidget>("AyysKing");
