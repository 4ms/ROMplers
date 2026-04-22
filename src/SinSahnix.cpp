#include "SinSahnixSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct SinSahnix : DrumMachineBaseModule<SinSahnix> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", SSKick, 2.f},
		NamedSample{"Snare", SSSnare, 2.f},
		NamedSample{"Tom Lo", SSTomL, 2.f},
		NamedSample{"Tom Mid", SSTomM, 2.f},
		NamedSample{"Tom Hi", SSTomH, 2.f},
		NamedSample{"Cymbal", SSCymbal, 2.f},
	};
};

struct SinSahnixWidget : DrumMachineVerticalBase<SinSahnixWidget, SinSahnix> {
	static constexpr auto panel = "res/panels/SinSahnix.svg";
	using DrumMachineVerticalBase::DrumMachineVerticalBase;
};

Model *modelSinSahnix = createModel<SinSahnix, SinSahnixWidget>("SinSahnix");
