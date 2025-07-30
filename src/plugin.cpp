#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelKayOne);
	p->addModel(modelKayArr);
	p->addModel(modelAyysKing);
	p->addModel(modelSinSahnix);
	p->addModel(modelSicksOh);
	p->addModel(modelDeeArr);
	p->addModel(modelSeaArr);
	p->addModel(modelSehvenToo);
	p->addModel(modelKick);
	p->addModel(modelSnare);
	p->addModel(modelTom);
	p->addModel(modelPercussion);
	p->addModel(modelRide);
	p->addModel(modelRimshot);
	p->addModel(modelClap);
	p->addModel(modelCrash);
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
