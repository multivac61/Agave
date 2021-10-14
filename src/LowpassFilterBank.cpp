// THIS MODULE IMPLEMENTS A STATIC LOWPASS FILTER BANK. RECOMMENDED FOR
// WIDEBAND INPUTS, SUCH AS WHITE/PINK/RED/ETC NOISE
// 
// 
// THIS CODE IS PROVIDED "AS-IS", WITH NO GUARANTEE OF ANY KIND.
// 
// CODED BY F. ESQUEDA - JANUARY 2018
#include <iostream>
#include <array>

#include "Agave.hpp"
#include "dsp/Filters.hpp"
#include "Components.hpp"

struct LowpassFilterBank : Module {

	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		FILTER_LOW_OUTPUT,
		FILTER_198_OUTPUT,
		FILTER_373_OUTPUT,
		FILTER_692_OUTPUT,
		FILTER_1411_OUTPUT,
		FILTER_HIGH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float sampleRate = APP->engine->getSampleRate();

	// Declare array of filters
	std::array<RCFilter, NUM_OUTPUTS> filters;

	// In Hz
	std::array<float, NUM_OUTPUTS> cutoffFrequencies = {{78.0f, 198.0f, 373.0f, 692.0f, 1411.0f, 3.0e3f}};

	// Derived class constructor
	LowpassFilterBank() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(SIGNAL_INPUT, "Signal");
        configOutput(FILTER_LOW_OUTPUT, "Low frequency");
        configOutput(FILTER_198_OUTPUT, "198 Hz");
        configOutput(FILTER_373_OUTPUT, "373 Hz");
        configOutput(FILTER_692_OUTPUT, "692 Hz");
        configOutput(FILTER_1411_OUTPUT, "1411 Hz");
        configOutput(FILTER_HIGH_OUTPUT, "High frequency");
		
		// Initialize filters
		auto *fc = begin(cutoffFrequencies);
		for (auto &filter : filters) {
			filter = RCFilter(*fc, sampleRate);
			if (fc != end(cutoffFrequencies))
				++fc;
		}

	}

	void step() override;
	void onSampleRateChange() override;
	// TODO: reset()

};


void LowpassFilterBank::step() {

	// Read input sample
	float input = inputs[SIGNAL_INPUT].value;

	// Send input to all filters
	int idx = 0;
	for (auto &filter : filters) {
		filter.process(input);
		outputs[idx].value = filter.getLowpassOutput();
		if (idx != NUM_OUTPUTS)
			++idx;
	}

}

void LowpassFilterBank::onSampleRateChange() {

	for(auto &filter : filters)
		filter.setSampleRate(APP->engine->getSampleRate());

}

struct LowpassFilterBankWidget : ModuleWidget {
    LowpassFilterBankWidget(LowpassFilterBank* module);
};

namespace Comps = AgaveComponents;
LowpassFilterBankWidget::LowpassFilterBankWidget(LowpassFilterBank* module) {
	setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LPFBank.svg")));
    Comps::createScrews<Comps::ScrewMetal>(*this);

	// SIGNAL INPUT
	addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 22.5)), module, LowpassFilterBank::SIGNAL_INPUT));

	// FILTERED OUTPUTS
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 40.0)), module, LowpassFilterBank::FILTER_LOW_OUTPUT));
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 52.5)), module, LowpassFilterBank::FILTER_198_OUTPUT));
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 65.0)), module, LowpassFilterBank::FILTER_373_OUTPUT));
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 77.5)), module, LowpassFilterBank::FILTER_692_OUTPUT));
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 90.0)), module, LowpassFilterBank::FILTER_1411_OUTPUT));
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 102.5)), module, LowpassFilterBank::FILTER_HIGH_OUTPUT));
}

Model* modelLowpassFilterBank = createModel<LowpassFilterBank, LowpassFilterBankWidget>("LowpassFilterBank");
