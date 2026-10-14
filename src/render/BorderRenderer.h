#pragma once
#include "../model/Composition.h"
#include "../geometry/BorderMesh.h"
#include "ShaderManager.h"
#include "GlowPass.h"
#include <unordered_map>
namespace sb {
class BorderRenderer {
public:
    void setup();
    void render(const Composition&,const Project&,float time);
    ofFbo& frame(){return target;}
    ofFbo& preview(){return display;}
    void clearCache(){cache.clear();}
    std::string error;
    size_t vertices=0, regenerations=0;
private:
    struct Cached { std::string key; BorderMesh border; };
    std::unordered_map<std::string,Cached> cache;
    ofFbo target,layer,backup,display,bloomSource;
    ofShader composite,previewShader;
    ShaderManager shaders;
    GlowPass glow;
    void allocate(int width,int height);
    void drawSlice(const BorderMesh&,const ofJson&,float time,float phase=0,float gain=1);
};
}
