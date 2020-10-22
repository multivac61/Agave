// 
// 
// THIS CODE IS PROVIDED "AS-IS", WITH NO GUARANTEE OF ANY KIND.
// 
// CODED BY F. ESQUEDA - JANUARY 2018
#include <iostream>
#include <iomanip>

#include "Agave.hpp"
#include "dsp/MS20Filter.hpp"
#include "Components.hpp"

namespace {
    constexpr float minCutoff = 50.0;
	constexpr float maxCutoff = 15.0e3;
}

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
    dsp::ClockDivider paramDivider;

	MS20VCF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_PARAM, 0.f, 1.f, 0.5f, "Frequency", " Hz", maxCutoff / minCutoff, minCutoff);
        configParam(CV_ATT_PARAM, -1.0f, 1.0f, 0.0f, "CV Attenuverter");
        configParam(RES_PARAM, 0.f, 2.f, 0.0f, "Resonance");

        paramDivider.setDivision (16);
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

    if(paramDivider.process()) {
	    // Set cutoff frequency
	    float cutoffCV = params[FREQ_PARAM].value + params[CV_ATT_PARAM].value*inputs[FREQ_CV_PARAM].value*0.2f;
	    cutoffCV = clamp(cutoffCV, 0.0f, 1.0f);
	    float fc = minCutoff * powf(maxCutoff / minCutoff, cutoffCV);

	    // Read resonance
	    float k = params[RES_PARAM].value;

        filter.setParams(fc, k);
    }

	// Push samples into the filter
	filter.process(input);

	// Send samples to output buffer 
	outputs[SIGNAL_OUTPUT].value = 1.0f * filter.getOutput();

}

void MS20VCF::onSampleRateChange() {
	filter.setSampleRate(APP->engine->getSampleRate());
}

struct MS20VCFWidget : ModuleWidget {
    MS20VCFWidget(MS20VCF* module);
};

namespace Comps = AgaveComponents;
MS20VCFWidget::MS20VCFWidget(MS20VCF* module) {
	setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MS20.svg")));
    Comps::createScrews<Comps::ScrewMetal>(*this);

    // AUDIO INPUT
	addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 21.25)), module, MS20VCF::SIGNAL_INPUT));

    // FREQUENCY PARAM
	addParam(createParamCentered<Comps::Knob>(mm2px(Vec(8.82, 37.5)), module, MS20VCF::FREQ_PARAM));
	addParam(createParamCentered<Comps::SmallKnob>(mm2px(Vec(10.16, 55.0)), module, MS20VCF::CV_ATT_PARAM));
	addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 63.0)), module, MS20VCF::FREQ_CV_PARAM));

    // Resonance PARAM
	addParam(createParamCentered<Comps::Knob>(mm2px(Vec(8.82, 80.0)), module, MS20VCF::RES_PARAM));

    // AUDIO OUTPUT
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 105.0)), module, MS20VCF::SIGNAL_OUTPUT));
}

Model* modelMS20VCF = createModel<MS20VCF, MS20VCFWidget>("MS20VCF");
