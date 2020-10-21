#pragma once
#include <rack.hpp>

extern Plugin* pluginInstance;

namespace AgaveComponents {

// KNOBS
struct Knob : rack::SvgKnob {
    Knob() {
        minAngle = -0.76 * M_PI;
        maxAngle = 0.76 * M_PI;
        shadow->opacity = 0;

        constexpr double swShift = 1.345;
        SvgWidget* dial = createWidget<SvgWidget>(Vec(0.0, 0.0));
        SvgWidget* base = createWidget<SvgWidget>(mm2px(Vec(swShift, swShift)));
        SvgWidget* cap  = createWidget<SvgWidget>(mm2px(Vec(swShift, swShift)));

        dial->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob_6mm_dial.svg")));
        base->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob_6mm_base.svg")));
        cap ->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob_6mm_top.svg")));
        
        addChildBottom(dial);
        addChildBottom(base);
        addChild(cap);

        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob_6mm_mid.svg")));
        sw->box.pos = Vec(swShift, swShift);
        fb->box.pos = Vec(swShift, swShift);
        tw->box.pos = Vec(swShift, swShift);
    }
};

struct SmallKnob : rack::SvgKnob {
    SmallKnob() {
        minAngle = -0.76 * M_PI;
        maxAngle = 0.76 * M_PI;
        shadow->opacity = 0;

        SvgWidget* base = createWidget<SvgWidget>(Vec(0.0, 0.0));
        base->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob_3mm_base.svg")));
        addChildBottom(base);

        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob_3mm_top.svg")));
    }
};

// SWITCHES
struct Button : rack::SvgSwitch {
    Button() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Button_false.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Button_true.svg")));
    }
};

struct Switch : rack::SvgSwitch {
    Switch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/switch_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/switch_1.svg")));
    }
};

struct TriSwitch : rack::SvgSwitch {
    TriSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/tri_switch_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/tri_switch_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/tri_switch_2.svg")));
    }
};

// PORTS
struct InputPort : rack::SvgPort {
    InputPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/I_jack.svg")));
    }
};

struct OutputPort : rack::SvgPort {
    OutputPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/O_jack.svg")));
    }
};

// SCREWS
struct ScrewLight : rack::SvgScrew {
    ScrewLight() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw_L.svg")));
    }
};

struct ScrewMetal : rack::SvgScrew {
    ScrewMetal() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw_M.svg")));
    }
};

struct ScrewDark : rack::SvgScrew {
    ScrewDark() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw_D.svg")));
    }
};

template<typename SCREW>
inline void createScrews(rack::ModuleWidget& widget) {
    const auto width = widget.box.size.x;
    const auto height = widget.box.size.y;
    constexpr float screwHeight = 15.0f;
    widget.addChild(createWidgetCentered<SCREW>(Vec(width / 2.0f, screwHeight / 2.0f)));
    widget.addChild(createWidgetCentered<SCREW>(Vec(width / 2.0f, height - screwHeight / 2.0f)));
}

} // namespace AgaveComponents
