#include "Agave.hpp"


// The plugin-wide instance of the Plugin class
Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelTestEngine);
	p->addModel(modelLowpassFilterBank);
	p->addModel(modelSharpWavefolder);
	p->addModel(modelMetallicNoise);
	p->addModel(modelMS20VCF);
    p->addModel(modelBlank);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
