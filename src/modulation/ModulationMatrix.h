#pragma once
#include "../model/Composition.h"
#include "AudioAnalyzer.h"
#include "BeatClock.h"
#include "Envelope.h"
#include <unordered_map>
namespace sb {
class ModulationMatrix {
public:
    Project apply(const Project&,const Composition&,double time,const AudioBands&,const BeatClock&,const std::unordered_map<std::string,float>& osc);
    void trigger(const std::string& id,double time){envelopes[id].trigger(time);}
    void gate(const std::string& id,bool on,double time,float attack=0.02f,float decay=0.3f,float sustain=0.5f){envelopes[id].gate(on,time,attack,decay,sustain);}
    std::string error;
private:
    std::unordered_map<std::string,Envelope> envelopes;
    std::unordered_map<std::string,float> smoothed;
    int64_t lastBeat=-1;
    uint64_t lastTrigger=0;
};
}
