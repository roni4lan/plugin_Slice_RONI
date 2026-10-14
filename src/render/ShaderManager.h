#pragma once
#include "ofMain.h"
#include <map>
namespace sb {
inline bool linkedShader(const ofShader& shader){GLint linked=GL_FALSE;if(shader.getProgram())glGetProgramiv(shader.getProgram(),GL_LINK_STATUS,&linked);return linked==GL_TRUE;}
class ShaderManager {
public:
    ofShader& get(const std::string& fragment){
        auto it=loaded.find(fragment);if(it!=loaded.end())return it->second;
        ofShader shader;if(!shader.load(ofToDataPath("shaders/border.vert"),ofToDataPath(fragment)) || !linkedShader(shader))throw std::runtime_error("Shader invalido: "+fragment);
        return loaded.emplace(fragment,std::move(shader)).first->second;
    }
    void clear(){loaded.clear();}
private:std::map<std::string,ofShader> loaded;
};
}
