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

#include "Components.hpp"
#include "dsp/Filters.hpp"
#include "dsp/Waveshaping.hpp"

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

    static const int MAX_POLY = 16;
    float sampleRate = APP->engine->getSampleRate();

    // Array of folders for each polyphonic channel
    std::array<Wavefolder, 4> folder[MAX_POLY];
    HardClipper clipper[MAX_POLY];

    static constexpr float dcFreq = 10.0f;
    DCBlocker dcBlocker[MAX_POLY];

    SharpWavefolder() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(SIGNAL_INPUT, "Signal");
        configInput(FOLD_CV_INPUT, "Fold CV");
        configInput(SYMM_CV_INPUT, "Symmetry CV");
        configOutput(FOLDED_OUTPUT, "Signal");
        configBypass(SIGNAL_INPUT, FOLDED_OUTPUT);

        configParam(FOLDS_PARAM, 0.9f, 10.f, 0.9f, "Folds", "x");
        configParam(FOLD_ATT_PARAM, -1.0f, 1.0f, 0.0f, "Folds CV attenuverter", "%", 0, 100);
        configParam(SYMM_PARAM, -5.0f, 5.0f, 0.0f, "Symmetry", "V");
        configParam(SYMM_ATT_PARAM, -1.0f, 1.0f, 0.0f, "Symmetry CV attenuverter", "%", 0, 100);

        // Initialize all filters
        for (int c = 0; c < MAX_POLY; c++) {
            dcBlocker[c].setSampleRate(sampleRate);
            dcBlocker[c].setFrequency(dcFreq);
        }
    }

    void process(const ProcessArgs& args) override {
        // Get number of polyphonic channels
        int channels = std::max({inputs[SIGNAL_INPUT].getChannels(),
                               inputs[FOLD_CV_INPUT].getChannels(),
                               inputs[SYMM_CV_INPUT].getChannels()});
        
        // Set output channels
        outputs[FOLDED_OUTPUT].setChannels(channels);

        for (int c = 0; c < channels; c++) {
            // Scale input to be within [-1 1]
            float input = 0.2f * inputs[SIGNAL_INPUT].getVoltage(c);

            // Read fold cv control
            float foldCV = inputs[FOLD_CV_INPUT].getPolyVoltage(c);
            float foldLevel = params[FOLDS_PARAM].getValue() + 
                            params[FOLD_ATT_PARAM].getValue() * std::abs(foldCV);
            foldLevel = clamp(foldLevel, -10.0f, 10.0f);

            // Read symmetry cv control
            float symmCV = inputs[SYMM_CV_INPUT].getPolyVoltage(c);
            float symmLevel = params[SYMM_PARAM].getValue() + 
                            0.5f * params[SYMM_ATT_PARAM].getValue() * symmCV;
            symmLevel = clamp(symmLevel, -5.0f, 5.0f);

            // Implement wavefolders
            float foldedOutput = input * foldLevel + symmLevel;
            for (int i = 0; i < 4; i++) {
                folder[c][i].process(foldedOutput);
                foldedOutput = folder[c][i].getFoldedOutput();
            }

            // Saturator
            clipper[c].process(foldedOutput);
            foldedOutput = clipper[c].getClippedOutput();

            // DC blocker and output
            dcBlocker[c].process(foldedOutput);
            outputs[FOLDED_OUTPUT].setVoltage(5.0f * dcBlocker[c].getFilteredOutput(), c);
        }
    }

    void onSampleRateChange() override {
        sampleRate = APP->engine->getSampleRate();
        for (int c = 0; c < MAX_POLY; c++) {
            dcBlocker[c].setSampleRate(sampleRate);
        }
    }

    void onReset() override {
        for (int c = 0; c < MAX_POLY; c++) {
            for (auto &f : folder[c]) {
                f.reset();
            }
            clipper[c].reset();
            dcBlocker[c].reset();
        }
    }
};

struct SharpWavefolderWidget : ModuleWidget {
    SharpWavefolderWidget(SharpWavefolder* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Fxld.svg")));
        
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
};

Model* modelSharpWavefolder = createModel<SharpWavefolder, SharpWavefolderWidget>("SharpWavefolder");
