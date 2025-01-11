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
        RES_CV_PARAM,
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

    // Replace single filter with array of filters for polyphony
    static const int MAX_POLY = 16;
    MS20Filter filters[MAX_POLY];
    dsp::ClockDivider paramDivider;

    MS20VCF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(SIGNAL_INPUT, "Signal");
        configInput(FREQ_CV_PARAM, "Frequency CV");
        configInput(RES_CV_PARAM, "Resonance CV");
        configOutput(SIGNAL_OUTPUT, "Signal");
        configBypass(SIGNAL_INPUT, SIGNAL_OUTPUT);

        configParam(FREQ_PARAM, 0.f, 1.f, 0.5f, "Frequency", " Hz", maxCutoff / minCutoff, minCutoff);
        configParam(CV_ATT_PARAM, -1.0f, 1.0f, 0.0f, "CV Attenuverter");
        configParam(RES_PARAM, 0.f, 2.f, 0.0f, "Resonance");

        paramDivider.setDivision(16);

        // Initialize all filters with current sample rate
        for (int i = 0; i < MAX_POLY; i++) {
            filters[i].setSampleRate(APP->engine->getSampleRate());
        }
    }

    void onSampleRateChange() override {
        for (int i = 0; i < MAX_POLY; i++)
            filters[i].setSampleRate(APP->engine->getSampleRate());
    }

    void onReset() override {
        for (int i = 0; i < MAX_POLY; i++)
            filters[i].reset();
    }

    void process(const ProcessArgs& args) override {
        // Get number of polyphonic channels from input
        int channels = std::max({inputs[SIGNAL_INPUT].getChannels(), inputs[FREQ_CV_PARAM].getChannels(), inputs[RES_CV_PARAM].getChannels()});

        // Set output channels to match input
        outputs[SIGNAL_OUTPUT].setChannels(channels);

        if (paramDivider.process()) {
            float baseFreq = params[FREQ_PARAM].getValue();
            float cvAtt = params[CV_ATT_PARAM].getValue();
            float resonance = params[RES_PARAM].getValue();

            // Process each channel
            for (int c = 0; c < channels; c++) {
                // Get CV for this channel (or use channel 0 if mono CV)
                float freqCV = inputs[FREQ_CV_PARAM].getPolyVoltage(c);
                float resCV = inputs[RES_CV_PARAM].getPolyVoltage(c);

                // Calculate cutoff frequency
                float cutoffCV = baseFreq + cvAtt * freqCV * 0.2f;
                cutoffCV = clamp(cutoffCV, 0.0f, 1.0f);
                float fc = minCutoff * powf(maxCutoff / minCutoff, cutoffCV);

                // Update filter parameters for this channel
                filters[c].setParams(fc, resonance + resCV);
            }
        }

        // Process audio for each channel
        for (int c = 0; c < channels; c++) {
            // Get input for this channel
            float input = inputs[SIGNAL_INPUT].getPolyVoltage(c);
            input = clamp(input, -6.0f, 6.0f);

            // Add noise to bootstrap self-oscillation
            input += 1.0e-2f * (2.0f * random::uniform() - 1.0f);

            // Original MS20 used 4.0V pkk
            input *= 1.0f * 0.2f;

            // Process through filter
            filters[c].process(input);

            // Set output for this channel
            outputs[SIGNAL_OUTPUT].setVoltage(5.0f * filters[c].getOutput(), c);
        }
    }
};

namespace Comps = AgaveComponents;

struct MS20VCFWidget : ModuleWidget {
    MS20VCFWidget(MS20VCF* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MS20.svg")));

        Comps::createScrews<Comps::ScrewMetal>(*this);

        // AUDIO INPUT
        addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 21.25)), module, MS20VCF::SIGNAL_INPUT));

        // FREQUENCY PARAM
        addParam(createParamCentered<Comps::Knob>(mm2px(Vec(8.82, 37.5)), module, MS20VCF::FREQ_PARAM));
        addParam(createParamCentered<Comps::SmallKnob>(mm2px(Vec(10.16, 55.0)), module, MS20VCF::CV_ATT_PARAM));
        addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 63.0)), module, MS20VCF::FREQ_CV_PARAM));

        // Resonance PARAM
        addParam(createParamCentered<Comps::Knob>(mm2px(Vec(8.82, 80.0)), module, MS20VCF::RES_PARAM));
        addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 93.0)), module, MS20VCF::RES_CV_PARAM));

        // AUDIO OUTPUT
        addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 105.0)), module, MS20VCF::SIGNAL_OUTPUT));
    }
};

Model* modelMS20VCF = createModel<MS20VCF, MS20VCFWidget>("MS20VCF");
