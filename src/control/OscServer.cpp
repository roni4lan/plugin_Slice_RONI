#include "OscServer.h"
#include "../model/StyleParams.h"
#include "../styles/StyleRegistry.h"
namespace sb { namespace {
float number(const ofxOscMessage& message,int index=0){
    if(index>=int(message.getNumArgs()))throw std::runtime_error("Falta argumento OSC");
    auto type=message.getArgType(index);
    if(type!=OFXOSC_TYPE_FLOAT && type!=OFXOSC_TYPE_DOUBLE && type!=OFXOSC_TYPE_INT32 && type!=OFXOSC_TYPE_INT64 && type!=OFXOSC_TYPE_TRUE && type!=OFXOSC_TYPE_FALSE)throw std::runtime_error("Argumento OSC no numerico");
    float v=message.getArgAsDouble(index);if(!std::isfinite(v))throw std::runtime_error("Argumento OSC no finito");return v;
}
std::string text(const ofxOscMessage& message){if(message.getNumArgs()==0 || message.getArgType(0)!=OFXOSC_TYPE_STRING)throw std::runtime_error("Se requiere texto OSC");return message.getArgAsString(0);}
void assign(ofJson& destination,const ofJson& effective,const std::string& target,const ofxOscMessage& message){
    if(colorComponent(target)>=0){setNumeric(destination,effective,target,number(message));return;}
    auto* parameter=findParameter(target);if(!parameter)throw std::runtime_error("Parametro OSC desconocido");
    if(parameter->type=="float" || parameter->type=="int")setNumeric(destination,effective,target,number(message));
    else if(parameter->type=="bool")destination[target]=number(message)!=0;
    else if(parameter->type=="color"){
        if(message.getNumArgs()!=4)throw std::runtime_error("Color requiere cuatro floats RGBA");
        destination[target]={ofClamp(number(message,0),0.f,1.f),ofClamp(number(message,1),0.f,1.f),ofClamp(number(message,2),0.f,1.f),ofClamp(number(message,3),0.f,1.f)};
    }else if(parameter->type=="enum"){
        if(message.getNumArgs()>0 && message.getArgType(0)==OFXOSC_TYPE_STRING){auto v=text(message);if(std::find(parameter->options.begin(),parameter->options.end(),v)==parameter->options.end())throw std::runtime_error("Opcion OSC no valida");destination[target]=v;}
        else {int index=int(number(message));if(index<0 || index>=int(parameter->options.size()))throw std::runtime_error("Indice OSC no valido");destination[target]=parameter->options[index];}
    }
}
}
void OscServer::update(Project& project,const Composition& composition,BeatClock& clock,ModulationMatrix& matrix,double time){
    bool enabled=project.osc.value("enabled",false);if(!enabled){if(port)stop();return;}
    try{
        int wanted=project.osc.value("port",7000);if(wanted<1 || wanted>65535)throw std::runtime_error("Puerto OSC invalido");
        if(port!=wanted){stop();ofxOscReceiverSettings settings;settings.port=wanted;settings.reuse=false;if(!receiver.setup(settings))throw std::runtime_error("No se pudo abrir el puerto OSC");port=wanted;status="OSC escuchando en "+ofToString(port);}
        ofxOscMessage message;int budget=64;
        while(budget-- && receiver.getNextMessage(message)){dispatch(message,project,composition,clock,matrix,time);++messages;}
    }catch(const std::exception& e){status=e.what();}
}
bool OscServer::dispatch(const ofxOscMessage& message,Project& project,const Composition& composition,BeatClock& clock,ModulationMatrix& matrix,double time){
    try{
        auto address=message.getAddress();if(address.size()>512 || message.getNumArgs()>16)throw std::runtime_error("Mensaje OSC demasiado largo");
        if(message.getNumArgs()>0){auto type=message.getArgType(0);if(type==OFXOSC_TYPE_FLOAT || type==OFXOSC_TYPE_INT32 || type==OFXOSC_TYPE_DOUBLE || type==OFXOSC_TYPE_INT64){if(values.size()<128 || values.count(address))values[address]=number(message);}}
        auto path=ofSplitString(address,"/",true,true);
        if(path.size()==2 && path[0]=="global"){
            auto effective=styleDefaults();effective.update(project.defaultStyle);assign(project.defaultStyle,effective,path[1],message);
        }else if(path.size()==3 && path[0]=="slice"){
            const Slice* slice=nullptr;for(const auto& screen:composition.screens)for(const auto& s:screen.slices)if(s.id==path[1])slice=&s;
            if(!slice)throw std::runtime_error("Slice OSC no encontrado");auto& binding=project.bindings[path[1]];if(!binding.is_object())binding={{"enabled",slice->enabled},{"params",ofJson::object()}};
            if(path[2]=="enabled")binding["enabled"]=number(message)!=0;
            else assign(binding["params"],resolveStyle(project.defaultStyle,project.groups,binding),path[2],message);
        }else if(path.size()==3 && path[0]=="group"){
            if(!project.groups.contains(path[1]))throw std::runtime_error("Grupo OSC no encontrado");auto effective=styleDefaults();effective.update(project.defaultStyle);effective.update(project.groups[path[1]]);assign(project.groups[path[1]],effective,path[2],message);
        }else if(path.size()==2 && path[0]=="style"){
            auto name=text(message);const auto& style=StyleRegistry::instance().get(name);bool found=false;for(const auto& screen:composition.screens)for(const auto& slice:screen.slices)if(slice.id==path[1])found=true;
            if(!found)throw std::runtime_error("Slice OSC no encontrado");auto& binding=project.bindings[path[1]];if(!binding.is_object())binding={{"enabled",true},{"params",ofJson::object()}};
            auto defaults=styleDefaults();size_t i=0;for(const auto& p:parameterRegistry()){if(i++<12)continue;binding["params"][p.name]=defaults[p.name];}binding["params"]["gradientStops"]=ofJson::array();binding["params"].update(style.defaults());binding["styleName"]=name;
        }else if(address=="/preset/load"){
            auto name=text(message);if(name.empty() || name.find('/')!=std::string::npos || name.find('\\')!=std::string::npos || name.find("..")!=std::string::npos)throw std::runtime_error("Nombre de preset invalido");
            if(ofFilePath::getFileExt(name)!="json")name+=".json";
            auto preset=ofLoadJson(ofToDataPath("presets/"+name)).at("params");validateStyle(preset);StyleRegistry::instance().get(preset.value("presetName",std::string("Solid")));
            project.defaultStyle=styleDefaults();project.defaultStyle.update(preset);
        }else if(address=="/clock/bpm"){
            double bpm=ofClamp(number(message),20.f,300.f);project.clock["bpm"]=bpm;project.defaultStyle["bpm"]=bpm;clock.setBpm(bpm,time);
        }else if(address=="/clock/tap"){double bpm=clock.tap(time);project.clock["bpm"]=bpm;project.defaultStyle["bpm"]=bpm;}
        else if(address=="/clock/beat")clock.beat(time);
        else if(address=="/clock/phase"){float phase=number(message);if(phase<0)throw std::runtime_error("Fase OSC negativa");clock.setBeats(phase,time);}
        else if(address=="/output/enable")project.output["enabled"]=number(message)!=0;
        else if(path.size()==2 && path[0]=="envelope")matrix.trigger(path[1],time);
        else if(path.size()==3 && path[0]=="envelope" && path[2]=="gate"){
            const ofJson* route=nullptr;for(const auto& r:project.modulation)if(r.value("id",std::string{})==path[1])route=&r;
            if(!route)throw std::runtime_error("Envolvente OSC no encontrada");matrix.gate(path[1],number(message)!=0,time,route->value("attack",0.02f),route->value("decay",0.3f),route->value("sustain",0.5f));
        }
        else if(path.size()==2 && path[0]=="mod"){number(message);}
        else if(values.count(address)==0)throw std::runtime_error("Direccion OSC no reconocida");
        status="OSC recibido: "+address;return true;
    }catch(const std::exception& e){status=e.what();return false;}
}
}
