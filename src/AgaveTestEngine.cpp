#include "Agave.hpp"
#include <iostream>

#include "dsp/DPWOsc.hpp"


struct AgaveTestEngine : Module {

	enum ParamIds {
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		SINE_OUTPUT,
		SAW_OUTPUT,
		SQUARE_OUTPUT,
		NOISE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float phase = 0.0;
	float blinkPhase = 0.0;

	DPWSawtooth sawtoothGenerator;

	DPWSquare squareWaveGenerator;

	AgaveTestEngine() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void AgaveTestEngine::step() {
	float SR = engineGetSampleRate();

	// Implement a simple sine oscillator
	float deltaTime = 1.0 / SR;

	// Compute the frequency from the pitch parameter and input
	float pitch = params[PITCH_PARAM].value;
	pitch = clampf(pitch, -15.0, 4.0);
	float freq = 440.0 * powf(2.0, pitch);

	// Accumulate the phase
	phase += freq * deltaTime;
	if (phase >= 1.0)
		phase -= 1.0;

	// Compute the sine output
	float sine = (sinf(2 * M_PI * phase));

	// Output sine
	outputs[SINE_OUTPUT].value = 5.0 * sine;

	// Output sawtooth
	sawtoothGenerator.generateSamples(freq);
	outputs[SAW_OUTPUT].value = 5.0 * sawtoothGenerator.output;

	// Output square wave
	squareWaveGenerator.generateSamples(freq);
	outputs[SQUARE_OUTPUT].value = 5.0 * squareWaveGenerator.output;

	// Generate white noise
	outputs[NOISE_OUTPUT].value = 5.0 * (2.0 * randomf() - 1.0);

}

AgaveTestEngineWidget::AgaveTestEngineWidget() {
	AgaveTestEngine *module = new AgaveTestEngine();
	setModule(module);
	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TestGeneratorPanel.svg")));
		addChild(panel);
	}

	// KNOB
	addParam(createParam<Davies1900hBlackKnob>(Vec(13, 87), module, AgaveTestEngine::PITCH_PARAM, -15.0, 4.0, -2.5));

	// SINE OUTPUT
	addOutput(createOutput<PJ301MPort>(Vec(18, 180), module, AgaveTestEngine::SINE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 230), module, AgaveTestEngine::SAW_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 280), module, AgaveTestEngine::SQUARE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(18, 330), module, AgaveTestEngine::NOISE_OUTPUT));

}
