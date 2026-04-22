#include "KayOneSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct KayOne : DrumMachineBaseModule<KayOne> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", SKKick, 2.f},
		NamedSample{"Snare", SKSnare, 2.f},
		NamedSample{"Tom Lo", SKTomLo, 2.f},
		NamedSample{"Tom Hi", SKTomHi, 2.f},
		NamedSample{"Closed Hat", SKClosedHat, 2.f},
		NamedSample{"Open Hat", SKOpenHat, 2.f},
	};
};

struct KayOneWidget : DrumMachineVerticalBase<KayOneWidget, KayOne> {
	static constexpr auto panel = "res/panels/KayOne.svg";
	using DrumMachineVerticalBase::DrumMachineVerticalBase;
};

Model *modelKayOne = createModel<KayOne, KayOneWidget>("KayOne");
