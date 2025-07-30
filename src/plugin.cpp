#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
p->addModel(modelAyysKing);
p->addModel(modelClap);
p->addModel(modelClosedHat);
p->addModel(modelCrash);
p->addModel(modelDeeArr);
p->addModel(modelKayArr);
p->addModel(modelKayOne);
p->addModel(modelKick);
p->addModel(modelOpenHat);
p->addModel(modelPercussion);
p->addModel(modelRide);
p->addModel(modelRimshot);
p->addModel(modelSeaArr);
p->addModel(modelSehvenToo);
p->addModel(modelSicksOh);
p->addModel(modelSinSahnix);
p->addModel(modelSnare);
p->addModel(modelTom);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
