#pragma once
#include "ofMain.h"

namespace sb {
struct Slice {
    std::string id, name;
    bool enabled = true;
    std::vector<glm::vec2> inputPoints, outputPoints, warpPoints;
    int inputOrientation = 0, outputOrientation = 0;
    int outputFlip = 0, warpFlip = 0;
    std::string inputSource, pointMode;
};
struct Screen {
    std::string id, name;
    int width = 0, height = 0;
    bool enabled = true, hidden = false;
    std::vector<Slice> slices;
};
struct Composition {
    int width = 0, height = 0;
    std::string arenaVersion;
    std::vector<Screen> screens;
};
// JSON-backed settings preserve additional parameters as the next phases evolve.
struct Project {
    std::string xmlPath;
    ofJson bindings = ofJson::object();
    ofJson screenEnabled = ofJson::object();
    ofJson groups = ofJson::object();
    ofJson defaultStyle = {{"thickness", 6.0}, {"colorA", {0.0, 0.9, 1.0, 1.0}}};
    ofJson output = {{"type", "Spout"}, {"name", "RONI RESO"}, {"enabled", false},
                     {"resolutionMode", "composition"}, {"fps", 60}};
    ofJson osc = {{"enabled", false}, {"port", 7000}};
    ofJson clock = {{"bpm",128.0},{"playing",true}};
    ofJson audio = {{"enabled",false},{"deviceIndex",0}};
    ofJson modulation = ofJson::array();
};
}
