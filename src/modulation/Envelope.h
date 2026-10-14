#pragma once
#include <algorithm>
#include <cmath>
namespace sb {
class Envelope {
public:
    void trigger(double time){start=time;releaseStart=-1;active=true;}
    void gate(bool on,double time,float attack,float decay,float sustain){if(on)trigger(time);else if(active){releaseValue=value(time,attack,decay,sustain,0.3f,true);releaseStart=time;}}
    float value(double time,float attack,float decay,float sustain,float release,bool adsr=false){
        if(!active)return 0;float t=float(time-start);attack=std::max(attack,0.001f);decay=std::max(decay,0.001f);
        if(releaseStart>=0){float v=releaseValue*std::max(0.f,1-float(time-releaseStart)/std::max(release,0.001f));if(v<=0)active=false;return v;}
        if(t<attack)return std::max(0.f,t/attack);
        if(t<attack+decay)return 1-(t-attack)/decay*(adsr?1-sustain:1);
        if(adsr)return sustain;active=false;return 0;
    }
private:double start=0,releaseStart=-1;float releaseValue=0;bool active=false;
};
}
