// THIS MODULE IMPLEMENTS A FOUR-STAGE WAVEFOLDER. THE FOLDING
// FUNCTION USED IS VERY SHARP, SIMILAR TO THAT OF THE BUCHLA 
// 259 TIMBRE SECTION. 
// 
// THIS CODE IS PROVIDED "AS-IS", WITH NO GUARANTEE OF ANY KIND.
// 
// CODED BY F. ESQUEDA - DECEMBER 2017
// 
// ADAPTED FOR VCV RACK JANUARY 2018
// 

#include "Agave.hpp"
#include <iostream>
#include <array>

#include "dsp/Waveshaping.hpp"
#include "Components.hpp"

struct SharpWavefolder : Module {

	enum ParamIds {
		FOLDS_PARAM,
		FOLD_ATT_PARAM,
		SYMM_PARAM,
		SYMM_ATT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_INPUT,
		FOLD_CV_INPUT,
		SYMM_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		FOLDED_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float sampleRate = APP->engine->getSampleRate();

	std::array<Wavefolder, 4> folder;

	HardClipper clipper;

	SharpWavefolder() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(SIGNAL_INPUT, "Signal");
        configInput(FOLD_CV_INPUT, "Fold CV");
        configInput(SYMM_CV_INPUT, "Symmetry CV");
        configOutput(FOLDED_OUTPUT, "Signal");

        configParam(FOLDS_PARAM, 0.9f, 10.f, 0.9f, "Folds");
        configParam(FOLD_ATT_PARAM, -1.0f, 1.0f, 0.0f, "Folds CV");
        configParam(SYMM_PARAM, -5.0f, 5.0f, 0.0f, "Symmetry");
        configParam(SYMM_ATT_PARAM, -1.0f, 1.0f, 0.0f, "Symmetry CV");
    }

	void step() override;
	void onSampleRateChange() override;

};

void SharpWavefolder::step() {

	// Scale input to be within [-1 1]
	float input = 0.2f * inputs[SIGNAL_INPUT].value;

	// Read fold cv control
	float foldLevel = params[FOLDS_PARAM].value + params[FOLD_ATT_PARAM].value*std::abs(inputs[FOLD_CV_INPUT].value);
	foldLevel = clamp(foldLevel, -10.0f, 10.0f);

	// Read symmetry cv control
	float symmLevel = params[SYMM_PARAM].value + 0.5f*params[SYMM_ATT_PARAM].value*inputs[SYMM_CV_INPUT].value;
	symmLevel = clamp(symmLevel, -5.0f, 5.0f);

	// Implement wavefolders
	float foldedOutput = input*foldLevel + symmLevel;
	for (int i=0; i<4; i++) {
		folder[i].process(foldedOutput);
		foldedOutput = folder[i].getFoldedOutput();
	}

	// Saturator
	clipper.process(foldedOutput);
	foldedOutput = clipper.getClippedOutput();

	// Send samples to output
	outputs[FOLDED_OUTPUT].value = 5.0f * foldedOutput;

}

void SharpWavefolder::onSampleRateChange() {
	sampleRate = APP->engine->getSampleRate();
}

struct SharpWavefolderWidget : ModuleWidget {
    SharpWavefolderWidget(SharpWavefolder* module);
};

namespace Comps = AgaveComponents;
SharpWavefolderWidget::SharpWavefolderWidget(SharpWavefolder* module) {
	setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Fxld.svg")));
    Comps::createScrews<Comps::ScrewMetal>(*this);

	// KNOBS
	addParam(createParamCentered<Comps::Knob>(mm2px(Vec(8.82, 32.5)), module, SharpWavefolder::FOLDS_PARAM));
	addParam(createParamCentered<Comps::SmallKnob>(mm2px(Vec(10.16, 50.0)), module, SharpWavefolder::FOLD_ATT_PARAM));
	addParam(createParamCentered<Comps::Knob>(mm2px(Vec(8.82, 72.5)), module, SharpWavefolder::SYMM_PARAM));
	addParam(createParamCentered<Comps::SmallKnob>(mm2px(Vec(10.16, 90.0)), module, SharpWavefolder::SYMM_ATT_PARAM));

	// IN JACKS
	addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 18.75)), module, SharpWavefolder::SIGNAL_INPUT));
	addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 58.0)), module, SharpWavefolder::FOLD_CV_INPUT));
	addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 98.0)), module, SharpWavefolder::SYMM_CV_INPUT));

	// OUT JACKS
	addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 110.0)), module, SharpWavefolder::FOLDED_OUTPUT));
}

Model* modelSharpWavefolder = createModel<SharpWavefolder, SharpWavefolderWidget>("SharpWavefolder");
