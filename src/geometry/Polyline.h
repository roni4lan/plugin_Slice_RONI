#pragma once
#include "ofMain.h"
namespace sb {
class Polyline {
public:
    static float length(const std::vector<glm::vec2>&, bool closed=true);
    static float area(const std::vector<glm::vec2>&);
    static std::vector<glm::vec2> resample(const std::vector<glm::vec2>&, float spacing, bool closed=true);
    static std::vector<glm::vec2> rounded(const std::vector<glm::vec2>&, float radius, int subdivision, bool closed=true);
    static std::vector<glm::vec2> offset(const std::vector<glm::vec2>&, float amount, float miterLimit, bool closed=true);
};
}
