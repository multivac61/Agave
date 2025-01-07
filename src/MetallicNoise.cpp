// THIS MODULE GENERATES "METALLIC NOISE" SIMILAR TO THAT USED IN THE 
// ROLAND TR-808 AND TR-606 RHYTHM COMPOSERS TO CREATE THE CYMBAL AND 
// HI-HAT VOICES. 
// 
// RECOMMENDED USE: METALLIC_NOISE -> HPF -> VCA WITH SNAPPY ENVELOPES
// 
// THIS CODE IS PROVIDED "AS-IS", WITH NO GUARANTEE OF ANY KIND.
// 
// CODED BY F. ESQUEDA - AUGUST 2017
// 
// ADAPTED FOR VCV RACK JANUARY 2018
// 
// TODO: 
//      ADD MORE NOISE SOURCES: DIGITAL NOISE, DR-110 & KR-55 NOISES

#include "Agave.hpp"
#include <iostream>
#include <array>

#include "dsp/DPWOsc.hpp"
#include "Components.hpp"

struct MetallicNoise : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        TRIG_INPUT, // Added trigger input for polyphonic triggering
        NUM_INPUTS
    };
    enum OutputIds {
        NOISE_808_OUTPUT,
        NOISE_606_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    static const int MAX_POLY = 16;
    float sampleRate = APP->engine->getSampleRate();

    // Declare 2 arrays of oscillators for each polyphonic channel
    std::array<DPWSquare, 6> squareWaves808[MAX_POLY];
    std::array<DPWSquare, 6> squareWaves606[MAX_POLY];

    // Define fundamental frequencies
    std::array<float, 6> oscFrequencies808 = {{205.3f, 369.4f, 304.4f, 522.3f, 800.0f, 540.4f}};
    std::array<float, 6> oscFrequencies606 = {{244.4f, 304.6f, 364.5f, 412.1f, 432.4f, 604.1f}};

    MetallicNoise() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(TRIG_INPUT, "Trigger");
        configOutput(NOISE_808_OUTPUT, "808");
        configOutput(NOISE_606_OUTPUT, "606");

        // Initialize all oscillators for all channels
        for (int c = 0; c < MAX_POLY; c++) {
            for (auto &squareWave : squareWaves808[c])
                squareWave.setSampleRate(sampleRate);
            for (auto &squareWave : squareWaves606[c])
                squareWave.setSampleRate(sampleRate);
        }
    }

    void process(const ProcessArgs& args) override {
        // Get number of polyphonic channels from trigger input
        int channels = inputs[TRIG_INPUT].getChannels();
        
        // Set output channels
        outputs[NOISE_808_OUTPUT].setChannels(channels);
        outputs[NOISE_606_OUTPUT].setChannels(channels);

        // Process each channel
        for (int c = 0; c < channels; c++) {
            // 808 Noise
            float output808 = 0.0f;
            for (int i = 0; i < 6; i++) {
                squareWaves808[c][i].generateSamples(oscFrequencies808[i]);
                output808 += squareWaves808[c][i].getSquareWaveform();
            }
            outputs[NOISE_808_OUTPUT].setVoltage(5.0f * 0.1666f * output808, c);

            // 606 Noise
            float output606 = 0.0f;
            for (int i = 0; i < 6; i++) {
                squareWaves606[c][i].generateSamples(oscFrequencies606[i]);
                output606 += squareWaves606[c][i].getSquareWaveform();
            }
            outputs[NOISE_606_OUTPUT].setVoltage(5.0f * 0.1666f * output606, c);
        }
    }

    void onSampleRateChange() override {
        for (int c = 0; c < MAX_POLY; c++) {
            for (auto &squareWave : squareWaves808[c])
                squareWave.setSampleRate(APP->engine->getSampleRate());
            for (auto &squareWave : squareWaves606[c])
                squareWave.setSampleRate(APP->engine->getSampleRate());
        }
    }

    void onReset() override {
        for (int c = 0; c < MAX_POLY; c++) {
            for (auto &squareWave : squareWaves808[c])
                squareWave.reset();
            for (auto &squareWave : squareWaves606[c])
                squareWave.reset();
        }
    }
};

namespace Comps = AgaveComponents;

struct MetallicNoiseWidget : ModuleWidget {
    MetallicNoiseWidget(MetallicNoise* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Metallic.svg")));
        
        Comps::createScrews<Comps::ScrewMetal>(*this);

        // TRIGGER INPUT
        addInput(createInputCentered<Comps::InputPort>(mm2px(Vec(10.16, 21.25)), module, MetallicNoise::TRIG_INPUT));

        // NOISE OUTPUTS
        addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 40.0)), module, MetallicNoise::NOISE_606_OUTPUT));
        addOutput(createOutputCentered<Comps::OutputPort>(mm2px(Vec(10.16, 80.0)), module, MetallicNoise::NOISE_808_OUTPUT));
    }
};

Model* modelMetallicNoise = createModel<MetallicNoise, MetallicNoiseWidget>("MetallicNoise");
