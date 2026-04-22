#include "DeeArrSamples.hpp"
#include "drum_machine_base.hh"
#include "plugin.hpp"

struct DeeArr : DrumMachineBaseModule<DeeArr> {
	static constexpr auto drums = std::array{
		NamedSample{"Kick", DAKick, 2.5f},
		NamedSample{"Snare", DASnare, 2.5f},
		NamedSample{"Hat", DAHat, 2.5f},
		NamedSample{"Rim", DARim, 2.5f},
	};
};

struct DeeArrWidget : DrumMachineVerticalBase<DeeArrWidget, DeeArr> {
	static constexpr auto panel = "res/panels/DeeArr.svg";
	using DrumMachineVerticalBase::DrumMachineVerticalBase;
};

Model *modelDeeArr = createModel<DeeArr, DeeArrWidget>("DeeArr");
