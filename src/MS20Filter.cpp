// 
// 
// THIS CODE IS PROVIDED "AS-IS", WITH NO GUARANTEE OF ANY KIND.
// 
// CODED BY F. ESQUEDA - JANUARY 2018
#include <iostream>
#include <iomanip>

#include "Agave.hpp"
#include "dsp/MS20Filter.hpp"

struct MS20VCF : Module {

	enum ParamIds {
		FREQ_PARAM,
		CV_ATT_PARAM,
		RES_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_INPUT,
		FREQ_CV_PARAM,
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float sampleRate = APP->engine->getSampleRate();

	MS20Filter filter{sampleRate};

	MS20VCF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_PARAM, 0.f, 1.f, 0.5f, "Frequency");
        configParam(CV_ATT_PARAM, -1.0f, 1.0f, 0.0f, "CV Attenuverter");
        configParam(RES_PARAM, 0.f, 2.f, 0.0f, "Resonance");
    }

	void step() override;
	void onSampleRateChange() override;
	void onReset() override {
		filter.reset();
	}

};


void MS20VCF::step() {

	// Expects inputs between [-5, 5] but we allow a bit extra ;-)
	float input = inputs[SIGNAL_INPUT].value;
	input = clamp(input, -6.0f, 6.0f);

	// Add noise to bootstrap self-oscillation
	input += 1.0e-2f * (2.0f*random::uniform() - 1.0f);

	// Original MS20 used 4.0V pkk
	input *= 1.0f * 0.2f;

	// Set cutoff frequency
	float cutoffCV = params[FREQ_PARAM].value + params[CV_ATT_PARAM].value*inputs[FREQ_CV_PARAM].value*0.2f;
	cutoffCV = clamp(cutoffCV, 0.0f, 1.0f);
	const float minCutoff = 50.0;
	const float maxCutoff = 15.0e3;
	float fc = minCutoff * powf(maxCutoff / minCutoff, cutoffCV);

	// Read resonance
	float k = params[RES_PARAM].value;

	// Push samples into the filter
	filter.process(input, fc, k);

	// Send samples to output buffer 
	outputs[SIGNAL_OUTPUT].value = 1.0f * filter.getOutput();

}

void MS20VCF::onSampleRateChange() {
	filter.setSampleRate(APP->engine->getSampleRate());
}

struct MS20VCFWidget : ModuleWidget {
    MS20VCFWidget(MS20VCF* module);
};

MS20VCFWidget::MS20VCFWidget(MS20VCF* module) {
	setModule(module);
	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TestGeneratorPanel.svg")));

	addInput(createInput<PJ301MPort>(Vec(18, 20), module, MS20VCF::SIGNAL_INPUT));

	addParam(createParam<Davies1900hBlackKnob>(Vec(12, 70), module, MS20VCF::FREQ_PARAM));
	addParam(createParam<Trimpot>(Vec(20, 120), module, MS20VCF::CV_ATT_PARAM));

	addInput(createInput<PJ301MPort>(Vec(18, 150), module, MS20VCF::FREQ_CV_PARAM));

	addParam(createParam<Davies1900hBlackKnob>(Vec(12, 200), module, MS20VCF::RES_PARAM));

	addOutput(createOutput<PJ301MPort>(Vec(18, 340), module, MS20VCF::SIGNAL_OUTPUT));
}

Model* modelMS20VCF = createModel<MS20VCF, MS20VCFWidget>("MS20VCF");
