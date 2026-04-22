#include "SicksOhSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SicksOh : DrumMachineBaseModule<SicksOh> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", SOKick, 2.5f},
		NamedSample{"Snare", SOSnare, 2.5f},
		NamedSample{"Tom Lo", SOTomL, 2.5f},
		NamedSample{"Tom Hi", SOTomH, 2.5f},
		NamedSample{"Closed Hat", SOClosedHat, 2.5f},
		NamedSample{"Open Hat", SOOpenHat, 2.5f},
		NamedSample{"Cymbal", SOCym, 2.5f},
	};
};

struct SicksOhWidget : DrumMachineVerticalBase<SicksOhWidget, SicksOh> {
	static constexpr auto panel = "res/panels/SicksOh.svg";
	using DrumMachineVerticalBase::DrumMachineVerticalBase;
};

Model *modelSicksOh = createModel<SicksOh, SicksOhWidget>("SicksOh");
