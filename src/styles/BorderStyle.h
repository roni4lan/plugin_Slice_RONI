#pragma once
#include "ofMain.h"
namespace sb {
class BorderStyle {
public:
    virtual ~BorderStyle()=default;
    virtual std::string name() const=0;
    virtual ofJson defaults() const=0;
    virtual std::string fragmentShader() const{return "shaders/border_solid.frag";}
    virtual void uniforms(ofShader&,const ofJson&,float) const{}
};
}
