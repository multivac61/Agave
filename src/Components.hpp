#pragma once
#include <rack.hpp>

using namespace rack;
extern Plugin* pluginInstance;

struct DarkPort : app::SvgPort {
    DarkPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/port_dark.svg")));
    }
};

struct LightPort : app::SvgPort {
    LightPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/port_light.svg")));
    }
};
