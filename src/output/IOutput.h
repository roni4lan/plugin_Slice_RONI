#pragma once
#include "ofMain.h"
namespace sb {
class IOutput {
public:
    virtual ~IOutput()=default;
    virtual void setup(const std::string& name,int fps)=0;
    virtual bool send(ofTexture&)=0;
    virtual void close()=0;
    std::string status;
};
}
