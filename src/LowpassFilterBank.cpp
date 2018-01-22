// THIS MODULE IMPLEMENTS A STATIC LOWPASS FILTER BANK. RECOMMENDED FOR
// WIDEBAND INPUTS, SUCH AS WHITE/PINK/RED/ETC NOISE
// 
// 
// THIS CODE IS PROVIDED "AS-IS", WITH NO GUARANTEE OF ANY KIND.
// 
// CODED BY F. ESQUEDA - JANUARY 2018

#include "Agave.hpp"
#include <iostream>

#include "dsp/Filters.hpp"

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

	float cutoffFrequencies[6] = {78.0, 198.0, 373.0, 692.0, 1411.0, 3.0e3};

	RCFilter filter[6];

	LowpassFilterBank() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < 6; i++) {
			filter[i] = RCFilter(cutoffFrequencies[i], engineGetSampleRate());
		}
	}

	void step() override;

	// TODO: reset()
	// TODO: onSampleRateChange()

};


void LowpassFilterBank::step() {

	// Read audio input
	// TODO: Add LED to check for high-hain input
	float input = inputs[SIGNAL_INPUT].value;

	filter[0].process(input, engineGetSampleRate());
	outputs[FILTER_LOW_OUTPUT].value = filter[0].getLowpassOutput();

	filter[1].process(input, engineGetSampleRate());
	outputs[FILTER_198_OUTPUT].value = filter[1].getLowpassOutput();

	filter[2].process(input, engineGetSampleRate());
	outputs[FILTER_373_OUTPUT].value = filter[2].getLowpassOutput();

	filter[3].process(input, engineGetSampleRate());
	outputs[FILTER_692_OUTPUT].value = filter[3].getLowpassOutput();

	filter[4].process(input, engineGetSampleRate());
	outputs[FILTER_1411_OUTPUT].value = filter[4].getLowpassOutput();

	filter[5].process(input, engineGetSampleRate());
	outputs[FILTER_HIGH_OUTPUT].value = filter[5].getLowpassOutput();

}

LowpassFilterBankWidget::LowpassFilterBankWidget() {
	LowpassFilterBank *module = new LowpassFilterBank();
	setModule(module);
	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TestGeneratorPanel.svg")));
		addChild(panel);
	}

	// SIGNAL INPUT
	addInput(createInput<PJ301MPort>(Vec(18, 20), module, LowpassFilterBank::SIGNAL_INPUT));

	// SINE OUTPUT
	addOutput(createOutput<PJ301MPort>(Vec(18, 80), module, LowpassFilterBank::FILTER_LOW_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 130), module, LowpassFilterBank::FILTER_198_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 180), module, LowpassFilterBank::FILTER_373_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 230), module, LowpassFilterBank::FILTER_692_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 280), module, LowpassFilterBank::FILTER_1411_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 330), module, LowpassFilterBank::FILTER_HIGH_OUTPUT));

}
