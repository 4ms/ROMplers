#include "SeaArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SeaArr : DrumMachineBaseModule<SeaArr> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", SAKick, 2.f},
		NamedSample{"Snare", SASnare, 2.f},
		NamedSample{"Hat", SAHiHat, 2.f},
		NamedSample{"Hat Metal", SAHiHatMetal, 2.f},
		NamedSample{"Rimshot", SARim, 2.f},
		NamedSample{"Cowbell", SACowbell, 2.f},
		NamedSample{"Conga", SACongaL, 2.f},
		NamedSample{"Bongo Lo", SABongoL, 2.f},
		NamedSample{"Bongo Hi", SABongoH, 2.f},
		NamedSample{"Tambourine", SATamb, 2.f},
		NamedSample{"Guiro", SAGuiro, 2.f},
		NamedSample{"Cymbal", SACym, 2.f},
	};
};

struct SeaArrWidget : DrumMachine12BaseWidget<SeaArrWidget, SeaArr> {
	static constexpr auto panel = "res/panels/SeaArr.svg";
	using DrumMachine12BaseWidget::DrumMachine12BaseWidget;
};

Model *modelSeaArr = createModel<SeaArr, SeaArrWidget>("SeaArr");
