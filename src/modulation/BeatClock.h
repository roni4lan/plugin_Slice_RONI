#pragma once
#include <algorithm>
#include <deque>
#include <cmath>
namespace sb {
class BeatClock {
public:
    double beats(double time)const{return anchorBeats+(playing?(time-anchorTime)*bpm/60:0);}
    void setBpm(double value,double time){value=std::clamp(value,20.,300.);if(value==bpm)return;anchorBeats=beats(time);anchorTime=time;bpm=value;}
    void setPlaying(bool value,double time){if(playing==value)return;anchorBeats=beats(time);anchorTime=time;playing=value;}
    void beat(double time){anchorBeats=std::floor(beats(time))+1;anchorTime=time;++triggerCount;}
    void setBeats(double value,double time){anchorBeats=std::max(0.,value);anchorTime=time;++triggerCount;}
    double tap(double time){if(lastTap>=0){double dt=time-lastTap;if(dt>=0.2 && dt<=3){intervals.push_back(dt);while(intervals.size()>4)intervals.pop_front();double sum=0;for(auto v:intervals)sum+=v;setBpm(60*intervals.size()/sum,time);}else intervals.clear();}lastTap=time;return bpm;}
    double bpm=128;
    bool playing=true;
    uint64_t triggerCount=0;
private:double anchorTime=0,anchorBeats=0,lastTap=-1;std::deque<double> intervals;
};
}
