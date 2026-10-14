#pragma once
#include "ofMain.h"
namespace sb {
class GlowPass {
public:
    void setup();
    void apply(ofFbo& source,ofFbo& borders,ofFbo& destination,float radius,int passes);
private:ofShader blur,mix;ofFbo ping,pong;
};
}
