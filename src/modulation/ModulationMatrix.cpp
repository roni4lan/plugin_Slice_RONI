#include "ModulationMatrix.h"
#include "LFO.h"
#include "../model/StyleParams.h"
#include <map>
#include <tuple>
namespace sb {
Project ModulationMatrix::apply(const Project& base,const Composition& composition,double time,const AudioBands& audio,const BeatClock& clock,const std::unordered_map<std::string,float>& osc){
    Project result=base;error.clear();int64_t beat=std::floor(clock.beats(time));bool newBeat=beat!=lastBeat || clock.triggerCount!=lastTrigger;lastBeat=beat;lastTrigger=clock.triggerCount;
    result.defaultStyle["_beatCount"]=clock.beats(time);result.defaultStyle["_clockBpm"]=clock.bpm;
    auto phase=osc.find("/clock/phase");result.defaultStyle["_oscPhase"]=phase==osc.end()?clock.beats(time):double(phase->second);
    std::map<std::tuple<std::string,std::string,std::string>,float> deltas;
    for(const auto& route:base.modulation){
        try{
            if(!route.value("enabled",true))continue;
            auto id=route.at("id").get<std::string>(),source=route.value("source",std::string("LFO")),scope=route.value("scope",std::string("global")),target=route.at("target").get<std::string>(),bindingId=route.value("bindingId",std::string{});
            auto frequency=route.value("frequency",1.f),phase=route.value("phase",0.f);float signal=0;
            if(source=="LFO")signal=LFO::value(time,frequency,route.value("shape",std::string("sine")),phase,route.value("seed",1));
            else if(source=="Beat")signal=LFO::value(clock.beats(time),frequency,route.value("shape",std::string("sine")),phase);
            else if(source=="Audio"){
                float level=audio.levels[AudioAnalyzer::bandIndex(route.value("audioBand",std::string("bass")))];float smoothing=ofClamp(route.value("smoothing",0.7f),0.f,0.999f);
                signal=smoothed[id]=smoothed[id]*smoothing+level*(1-smoothing);
            }else if(source=="OSC"){auto key=route.value("sourceKey",std::string("/mod/1"));auto it=osc.find(key);signal=it==osc.end()?0:it->second;}
            else if(source=="Envelope"){
                auto& envelope=envelopes[id];if(newBeat && route.value("triggerOnBeat",true))envelope.trigger(time);
                signal=envelope.value(time,route.value("attack",0.02f),route.value("decay",0.3f),route.value("sustain",0.5f),route.value("release",0.3f),route.value("envelopeMode",std::string("AD"))=="ADSR");
            }else throw std::runtime_error("Fuente de modulacion desconocida");
            auto curve=route.value("curve",std::string("linear"));if(curve=="square")signal=std::copysign(signal*signal,signal);if(curve=="smooth")signal=std::copysign(std::abs(signal)*std::abs(signal)*(3-2*std::abs(signal)),signal);
            float delta=signal*route.value("amount",10.f)+route.value("offset",0.f);
            deltas[{scope,bindingId,target}]+=delta;
        }catch(const std::exception& e){error=e.what();}
    }
    // Sum all routes before clamping. Apply scopes in inheritance order.
    for(const auto& accumulated:deltas){
        try{
            const auto& scope=std::get<0>(accumulated.first);const auto& bindingId=std::get<1>(accumulated.first);const auto& target=std::get<2>(accumulated.first);
            ofJson* values=nullptr;auto effective=styleDefaults();effective.update(result.defaultStyle);
            if(scope=="global")values=&result.defaultStyle;
            else if(scope=="group"){
                if(!result.groups.contains(bindingId))throw std::runtime_error("Grupo no encontrado");values=&result.groups[bindingId];effective.update(*values);
            }else if(scope=="slice"){
                bool found=false;for(const auto& screen:composition.screens)for(const auto& slice:screen.slices)if(slice.id==bindingId)found=true;
                if(!found)throw std::runtime_error("Slice no encontrado");auto& binding=result.bindings[bindingId];if(!binding.is_object())binding={{"enabled",true},{"params",ofJson::object()}};
                effective=resolveStyle(result.defaultStyle,result.groups,binding);values=&binding["params"];
            }else throw std::runtime_error("Destino de modulacion desconocido");
            setNumeric(*values,effective,target,numericValue(effective,target)+accumulated.second);
        }catch(const std::exception& e){error=e.what();}
    }
    for(const auto& screen:composition.screens)for(const auto& slice:screen.slices){
        auto& binding=result.bindings[slice.id];if(!binding.is_object())binding={{"enabled",slice.enabled},{"params",ofJson::object()}};
        auto effective=resolveStyle(result.defaultStyle,result.groups,binding);
        if(effective["syncMode"]=="audio"){
            auto band=effective["audioBand"].get<std::string>();float amount=effective["audioAmount"],level=audio.levels[AudioAnalyzer::bandIndex(band)],smoothing=effective["audioSmoothing"];
            auto key="sync-"+slice.id;float smooth=smoothed[key]=smoothed[key]*smoothing+level*(1-smoothing);
            binding["params"]["opacity"]=effective["opacity"].get<float>()*ofClamp(1-amount+amount*smooth,0.f,1.f);
        }
    }
    return result;
}
}
