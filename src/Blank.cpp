#include "Agave.hpp"
#include "Components.hpp"

struct Blank : Module {
    enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		OUTPUT_LIGHT,
		NUM_LIGHTS
	};

    Blank() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
};

namespace Comps = AgaveComponents;
struct BlankWidget : ModuleWidget {
    BlankWidget(Blank* module) {
        setModule (module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Blank.svg")));
        Comps::createScrews<Comps::ScrewMetal>(*this);
    }
};

Model* modelBlank = createModel<Blank, BlankWidget>("AgaveBlank");
