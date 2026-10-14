#pragma once
#include <cmath>
#include <string>
namespace sb {
struct LFO {
    static float value(double time,float frequency,const std::string& shape,float phase=0,int seed=1){
        double cycle=time*frequency+phase,t=cycle-std::floor(cycle);
        if(shape=="triangle")return 1-4*std::abs(float(t)-0.5f);
        if(shape=="square")return t<0.5?1:-1;
        if(shape=="saw")return float(t)*2-1;
        if(shape=="random"){double h=std::sin(std::floor(cycle)*127.1+seed*17.71)*43758.5453;return (h-std::floor(h))*2-1;}
        return std::sin(cycle*6.28318530718);
    }
};
}
