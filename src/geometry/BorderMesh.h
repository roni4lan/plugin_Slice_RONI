#pragma once
#include "ofMain.h"
namespace sb {
struct BorderMesh {
    ofVboMesh mesh;
    std::vector<glm::vec2> contour;
    float perimeter=0;
    static BorderMesh build(const std::vector<glm::vec2>&,const ofJson& params);
};
}
