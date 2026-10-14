#pragma once
#include "IOutput.h"
#if defined(TARGET_OSX) && __has_include("ofxSyphon.h")
#define SB_HAS_SYPHON 1
#include "ofxSyphon.h"
#endif
namespace sb {
class SyphonOutput : public IOutput {
public:
    void setup(const std::string& name,int)override;
    bool send(ofTexture&)override;
    void close()override;
private:
#ifdef SB_HAS_SYPHON
    std::unique_ptr<ofxSyphonServer> server;
#endif
};
}
