#include "ControlPanels.h"
#include "../model/StyleParams.h"
#include "imgui.h"
namespace sb { namespace {
bool choice(const char* label,std::string& value,std::initializer_list<const char*> options){bool changed=false;if(ImGui::BeginCombo(label,value.c_str())){for(auto option:options)if(ImGui::Selectable(option,value==option)){value=option;changed=true;}ImGui::EndCombo();}return changed;}
void scalar(ofJson& route,const char* field,const char* label,float fallback,float min,float max){float v=route.value(field,fallback);if(ImGui::SliderFloat(label,&v,min,max,"%.3f",ImGuiSliderFlags_AlwaysClamp|ImGuiSliderFlags_NoRoundToFormat))route[field]=v;}
}
void drawControlPanels(Project& project,const Composition& composition,const std::string& selected,AudioAnalyzer& audio,BeatClock& clock,ModulationMatrix& matrix,OscServer& osc){
    if(ImGui::CollapsingHeader("Reloj y sincronizacion")){
        float bpm=project.clock.value("bpm",128.f);if(ImGui::SliderFloat("BPM master",&bpm,20,300)){project.clock["bpm"]=bpm;project.defaultStyle["bpm"]=bpm;}
        if(ImGui::Button("Tap (T)")){bpm=clock.tap(ofGetElapsedTimef());project.clock["bpm"]=bpm;project.defaultStyle["bpm"]=bpm;}
        ImGui::SameLine();if(ImGui::Button("Beat"))clock.beat(ofGetElapsedTimef());
        bool playing=project.clock.value("playing",true);if(ImGui::Checkbox("Reloj activo",&playing))project.clock["playing"]=playing;
        ImGui::Text("Beat: %.2f",clock.beats(ofGetElapsedTimef()));
    }
    if(ImGui::CollapsingHeader("Audio")){
        static std::vector<ofSoundDevice> inputs;static bool enumerated=false;
        if(!enumerated || ImGui::Button("Actualizar entradas")){inputs=audio.devices();enumerated=true;}
        int selectedDevice=project.audio.value("deviceIndex",0);
        const char* label=selectedDevice>=0 && selectedDevice<int(inputs.size())?inputs[selectedDevice].name.c_str():"Sin entrada";
        if(ImGui::BeginCombo("Entrada",label)){for(size_t i=0;i<inputs.size();++i)if(ImGui::Selectable(inputs[i].name.c_str(),int(i)==selectedDevice))project.audio["deviceIndex"]=int(i);ImGui::EndCombo();}
        bool enabled=project.audio.value("enabled",false);if(ImGui::Checkbox("Capturar entrada",&enabled))project.audio["enabled"]=enabled;
        if(ImGui::Button("Reintentar audio"))project.audio["retry"]=ofGetSystemTimeMicros();
        ImGui::TextWrapped("%s",audio.status.c_str());auto bands=audio.bands();const char* names[]={"Graves","Medios bajos","Medios","Agudos","Todo"};for(int i=0;i<5;++i)ImGui::ProgressBar(bands.levels[i],ImVec2(-1,0),names[i]);
    }
    if(ImGui::CollapsingHeader("OSC")){
        bool enabled=project.osc.value("enabled",false);if(ImGui::Checkbox("Escuchar OSC",&enabled))project.osc["enabled"]=enabled;
        int port=project.osc.value("port",7000);if(ImGui::InputInt("Puerto OSC",&port))project.osc["port"]=ofClamp(port,1,65535);
        ImGui::TextWrapped("%s",osc.status.c_str());ImGui::Text("Mensajes: %llu",static_cast<unsigned long long>(osc.messages));
    }
    if(ImGui::CollapsingHeader("Matriz de modulacion")){
        if(ImGui::Button("Agregar ruta")){
            std::string id="route-"+ofToString(ofGetSystemTimeMicros());project.modulation.push_back({{"id",id},{"enabled",true},{"scope",selected.empty()?"global":"slice"},{"bindingId",selected},{"target","thickness"},{"source","LFO"},{"frequency",1.0},{"shape","sine"},{"amount",10.0},{"offset",0.0},{"curve","linear"}});
        }
        for(size_t i=0;i<project.modulation.size();++i){
            auto& route=project.modulation[i];auto id=route.value("id",ofToString(i));ImGui::PushID(id.c_str());
            if(ImGui::TreeNode("Ruta")){
                ImGui::TextWrapped("ID: %s",id.c_str());
                bool enabled=route.value("enabled",true);if(ImGui::Checkbox("Activa",&enabled))route["enabled"]=enabled;
                auto source=route.value("source",std::string("LFO"));if(choice("Fuente",source,{"LFO","Beat","Envelope","Audio","OSC"}))route["source"]=source;
                auto scope=route.value("scope",std::string("global"));if(choice("Nivel",scope,{"global","group","slice"}))route["scope"]=scope;
                auto binding=route.value("bindingId",std::string{});
                if(scope!="global" && ImGui::BeginCombo("Destino",binding.c_str())){
                    if(scope=="group"){for(const auto& group:project.groups.items())if(ImGui::Selectable(group.key().c_str(),binding==group.key()))route["bindingId"]=group.key();}
                    else for(const auto& screen:composition.screens)for(const auto& slice:screen.slices)if(ImGui::Selectable((slice.name+"##"+slice.id).c_str(),binding==slice.id))route["bindingId"]=slice.id;
                    ImGui::EndCombo();
                }
                auto target=route.value("target",std::string("thickness"));if(ImGui::BeginCombo("Parametro",target.c_str())){for(const auto& name:numericTargets())if(ImGui::Selectable(name.c_str(),target==name))route["target"]=name;ImGui::EndCombo();}
                scalar(route,"amount","Amplitud",10,-200,200);scalar(route,"offset","Offset",0,-200,200);
                auto curve=route.value("curve",std::string("linear"));if(choice("Curva",curve,{"linear","square","smooth"}))route["curve"]=curve;
                if(source=="LFO" || source=="Beat"){
                    scalar(route,"frequency",source=="Beat"?"Ciclos por beat":"Frecuencia Hz",1,0.001,20);scalar(route,"phase","Fase",0,0,1);
                    auto shape=route.value("shape",std::string("sine"));if(choice("Forma",shape,{"sine","triangle","square","saw","random"}))route["shape"]=shape;
                }else if(source=="Audio"){
                    auto band=route.value("audioBand",std::string("bass"));if(choice("Banda",band,{"bass","lowMid","mid","high","all"}))route["audioBand"]=band;scalar(route,"smoothing","Suavizado",0.7,0,0.999);
                }else if(source=="OSC"){
                    char key[256];std::snprintf(key,sizeof(key),"%s",route.value("sourceKey",std::string("/mod/1")).c_str());if(ImGui::InputText("Direccion fuente",key,sizeof(key)))route["sourceKey"]=std::string(key);
                }else if(source=="Envelope"){
                    auto mode=route.value("envelopeMode",std::string("AD"));if(choice("Envolvente",mode,{"AD","ADSR"}))route["envelopeMode"]=mode;
                    scalar(route,"attack","Attack (s)",0.02,0.001,5);scalar(route,"decay","Decay (s)",0.3,0.001,5);scalar(route,"sustain","Sustain",0.5,0,1);scalar(route,"release","Release (s)",0.3,0.001,5);
                    bool trigger=route.value("triggerOnBeat",true);if(ImGui::Checkbox("Disparar por beat",&trigger))route["triggerOnBeat"]=trigger;
                    if(ImGui::Button("Disparar"))matrix.trigger(id,ofGetElapsedTimef());
                    if(ImGui::Button("Gate ON"))matrix.gate(id,true,ofGetElapsedTimef(),route.value("attack",0.02f),route.value("decay",0.3f),route.value("sustain",0.5f));ImGui::SameLine();if(ImGui::Button("Gate OFF"))matrix.gate(id,false,ofGetElapsedTimef(),route.value("attack",0.02f),route.value("decay",0.3f),route.value("sustain",0.5f));
                }
                bool remove=ImGui::Button("Eliminar ruta");ImGui::TreePop();ImGui::PopID();if(remove){project.modulation.erase(i);break;}
            }else ImGui::PopID();
        }
        if(!matrix.error.empty())ImGui::TextWrapped("%s",matrix.error.c_str());
    }
}
}
