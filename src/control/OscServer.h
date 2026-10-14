#pragma once
#include "ofxOsc.h"
#include "../model/Composition.h"
#include "../modulation/BeatClock.h"
#include "../modulation/ModulationMatrix.h"
namespace sb {
class OscServer {
public:
    void update(Project&,const Composition&,BeatClock&,ModulationMatrix&,double time);
    void stop(){receiver.stop();port=0;status="OSC desactivado";}
    bool dispatch(const ofxOscMessage&,Project&,const Composition&,BeatClock&,ModulationMatrix&,double time);
    std::unordered_map<std::string,float> values;
    std::string status="OSC desactivado";
    uint64_t messages=0;
private:ofxOscReceiver receiver;int port=0;
};
}
