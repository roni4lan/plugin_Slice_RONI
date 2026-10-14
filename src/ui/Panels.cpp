#include "Panels.h"
#include "../model/StyleParams.h"
#include "imgui.h"
#include "../styles/StyleRegistry.h"
namespace sb {
void drawStylePanel(Project& project,const std::string& selected){
    static int scope=0;static char newGroup[128]={};
    ImGui::SeparatorText("Estilo del borde");
    if(selected.empty())scope=2;
    ImGui::Combo("Editar",&scope,"Slice\0Grupo\0Global\0");
    ofJson emptyBinding=ofJson::object();auto& binding=selected.empty()?emptyBinding:project.bindings[selected];
    if(!binding.is_object())binding={{"enabled",true},{"styleName","Solid"},{"params",ofJson::object()}};
    auto group=binding.value("group",std::string{});
    if(!selected.empty()){
        if(ImGui::BeginCombo("Grupo",group.empty()?"Sin grupo":group.c_str())){
            if(ImGui::Selectable("Sin grupo",group.empty())){binding["group"]="";group.clear();}
            for(auto& item:project.groups.items())if(ImGui::Selectable(item.key().c_str(),item.key()==group)){group=item.key();binding["group"]=group;}
            ImGui::EndCombo();
        }
        ImGui::InputText("Nuevo grupo",newGroup,sizeof(newGroup));
        if(ImGui::Button("Crear y asignar") && newGroup[0]){group=newGroup;project.groups[group]=ofJson::object();binding["group"]=group;newGroup[0]=0;}
    }
    if(scope==0 && selected.empty()){ImGui::TextWrapped("Selecciona un slice para editar sus overrides.");return;}
    if(scope==1 && group.empty()){ImGui::TextWrapped("Asigna un grupo al slice seleccionado.");return;}
    ofJson* target=scope==2?&project.defaultStyle:scope==1?&project.groups[group]:&binding["params"];
    auto effective=styleDefaults();effective.update(project.defaultStyle);
    if(scope!=2 && !group.empty())effective.update(project.groups[group]);
    if(scope==0)effective.update(*target);
    if(ImGui::BeginCombo("Estilo",effective.value("presetName",std::string("Solid")).c_str())){
        for(const auto& entry:StyleRegistry::instance().styles())if(ImGui::Selectable(entry.first.c_str(),effective.value("presetName",std::string("Solid"))==entry.first)){
            auto defaults=styleDefaults();size_t index=0;
            for(const auto& parameter:parameterRegistry()){if(index++<12)continue;(*target)[parameter.name]=defaults[parameter.name];}
            (*target)["gradientStops"]=ofJson::array();target->update(entry.second->defaults());
            if(scope==0)binding["styleName"]=entry.first;
            effective.update(*target);
        }
        ImGui::EndCombo();
    }
    static std::string presetStatus;
    if(ImGui::Button("Guardar preset")){
        auto dialog=ofSystemSaveDialog(ofToDataPath("presets/Custom.json",true),"Guardar preset");
        if(dialog.bSuccess){auto values=effective;try{validateStyle(values);if(!ofSavePrettyJson(dialog.getPath(),{{"name",ofFilePath::getBaseName(dialog.getPath())},{"params",values}}))throw std::runtime_error("No se pudo guardar");presetStatus="Preset guardado";}catch(const std::exception& e){presetStatus=e.what();}}
    }
    ImGui::SameLine();if(ImGui::Button("Cargar preset")){
        auto dialog=ofSystemLoadDialog("Cargar preset JSON");
        if(dialog.bSuccess)try{auto values=ofLoadJson(dialog.getPath()).at("params");validateStyle(values);StyleRegistry::instance().get(values.value("presetName",std::string("Solid")));auto defaults=styleDefaults();size_t index=0;for(const auto& parameter:parameterRegistry()){if(index++<12)continue;(*target)[parameter.name]=defaults[parameter.name];}(*target)["gradientStops"]=ofJson::array();target->update(values);effective.update(*target);presetStatus="Preset cargado";}catch(const std::exception& e){presetStatus=e.what();}
    }
    if(!presetStatus.empty())ImGui::TextWrapped("%s",presetStatus.c_str());
    ImGui::TextWrapped(scope==2?"Valores globales":"Desmarca Propio para heredar el valor.");
    ImGui::PushItemWidth(-1);
    for(const auto& p:parameterRegistry()){
        ImGui::PushID(p.name.c_str());
        bool own=scope==2 || target->contains(p.name);
        if(scope!=2){if(ImGui::Checkbox("Propio",&own)){if(own)(*target)[p.name]=effective[p.name];else target->erase(p.name);}ImGui::SameLine();}
        ImGui::TextUnformatted(p.label.c_str());
        auto v=effective[p.name];bool changed=false;ImGui::BeginDisabled(!own);
        if(p.type=="float"){float f=v;changed=ImGui::SliderFloat("##value",&f,p.min,p.max,"%.3f");v=f;}
        else if(p.type=="int"){int i=v;changed=ImGui::SliderInt("##value",&i,p.min,p.max);v=i;}
        else if(p.type=="bool"){bool b=v;changed=ImGui::Checkbox("##value",&b);v=b;}
        else if(p.type=="color"){float c[4]={v[0],v[1],v[2],v[3]};changed=ImGui::ColorEdit4("##value",c);v={c[0],c[1],c[2],c[3]};}
        else if(p.type=="enum"){
            auto current=v.get<std::string>();
            if(ImGui::BeginCombo("##value",current.c_str())){for(const auto& option:p.options)if(ImGui::Selectable(option.c_str(),current==option)){v=option;changed=true;}ImGui::EndCombo();}
        }
        if(changed)(*target)[p.name]=v;
        ImGui::EndDisabled();ImGui::PopID();
    }
    ImGui::SeparatorText("Paradas del gradiente (max. 8)");
    bool stopsOwn=scope==2 || target->contains("gradientStops");
    if(scope!=2 && ImGui::Checkbox("Gradiente propio",&stopsOwn)){if(stopsOwn)(*target)["gradientStops"]=effective["gradientStops"];else target->erase("gradientStops");}
    ImGui::BeginDisabled(!stopsOwn);
    auto stops=effective["gradientStops"];
    if(stops.size()<8 && ImGui::Button("Agregar parada")){stops.push_back({{"pos",stops.empty()?0.f:1.f},{"color",effective["colorB"]}});(*target)["gradientStops"]=stops;}
    for(size_t i=0;i<stops.size();++i){
        ImGui::PushID(int(i));float position=stops[i]["pos"];
        float lower=i==0?0.f:stops[i-1]["pos"].get<float>(),upper=i+1==stops.size()?1.f:stops[i+1]["pos"].get<float>();
        if(ImGui::SliderFloat("Posicion",&position,lower,upper))stops[i]["pos"]=position;
        float color[4]={stops[i]["color"][0],stops[i]["color"][1],stops[i]["color"][2],stops[i]["color"][3]};
        if(ImGui::ColorEdit4("Color",color))stops[i]["color"]={color[0],color[1],color[2],color[3]};
        bool remove=ImGui::Button("Quitar");ImGui::PopID();
        if(remove){stops.erase(i);break;}
    }
    if(stopsOwn && target->value("gradientStops",ofJson::array())!=stops)(*target)["gradientStops"]=stops;
    ImGui::EndDisabled();ImGui::PopItemWidth();
}
}
