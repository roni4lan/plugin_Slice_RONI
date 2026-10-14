#pragma once
#include "ofMain.h"
namespace sb {
struct Parameter {
    std::string name, label, type;
    ofJson initial;
    float min=0, max=1;
    std::vector<std::string> options;
};
inline const std::vector<Parameter>& parameterRegistry() {
    static const std::vector<Parameter> p={
        {"space","Espacio","enum","input",0,1,{"input","output"}},
        {"thickness","Grosor (px)","float",6.0,0.5,200},
        {"offset","Offset interior (px)","float",0.0,-200,200},
        {"alignment","Alineacion","enum","center",0,1,{"center","inside","outside"}},
        {"cornerRadius","Radio esquina (px)","float",0.0,0,200},
        {"joinStyle","Union","enum","miter",0,1,{"miter","round","bevel"}},
        {"miterLimit","Limite miter","float",4.0,1,10},
        {"capStyle","Extremos","enum","butt",0,1,{"butt","round","square"}},
        {"closePath","Cerrar contorno","bool",true},
        {"subdivision","Subdivisiones","int",16,1,64},
        {"segmentsMode","Lados","enum","all",0,1,{"all","perEdge","selectedEdges"}},
        {"edgeMask","Lados activos (bits)","int",15,0,15},
        {"colorA","Color A","color",{0.0,0.9,1.0,1.0}},
        {"colorB","Color B","color",{1.0,0.0,0.8,1.0}},
        {"colorMode","Modo color","enum","solid",0,1,{"solid","gradientPerimeter"}},
        {"gradientRotation","Rotacion gradiente (grados/s)","float",0.0,-360,360},
        {"gradientOffset","Fase gradiente","float",0.0,0,1},
        {"gradientRepeat","Repeticiones","float",1.0,0.1,20},
        {"hueShiftSpeed","Velocidad tono","float",0.0,-2,2},
        {"saturation","Saturacion","float",1.0,0,2},
        {"brightness","Brillo","float",1.0,0,2},
        {"opacity","Opacidad","float",1.0,0,1},
        {"blendMode","Mezcla","enum","alpha",0,1,{"alpha","add","screen","multiply"}},
        {"innerFillOpacity","Opacidad relleno","float",0.0,0,1},
        {"innerFillColor","Color relleno","color",{0.0,0.9,1.0,1.0}},
        {"profile","Perfil","enum","hard",0,1,{"hard","soft","gaussian","doubleLine","tripleLine"}},
        {"softness","Suavidad","float",0.1,0,1},
        {"coreWidth","Ancho nucleo","float",0.0,0,1},
        {"coreColor","Color nucleo","color",{1.0,1.0,1.0,1.0}},
        {"lineGap","Separacion lineas (px)","float",4.0,0,200},
        {"lineCount","Cantidad lineas","int",1,1,6},
        {"glowEnabled","Halo","bool",false},
        {"glowMode","Modo halo","enum","falloff",0,1,{"falloff","bloom"}},
        {"glowRadius","Radio halo (px)","float",30.0,0,300},
        {"glowIntensity","Intensidad halo","float",1.0,0,5},
        {"glowFalloff","Caida halo","float",3.0,0.1,10},
        {"glowColor","Color halo","color",{0.0,0.9,1.0,1.0}},
        {"glowUseBorderColor","Halo usa color del borde","bool",true},
        {"glowPasses","Pasadas blur","int",2,1,8},
        {"pattern","Patron","enum","solid",0,1,{"solid","dashed","dotted","segments","ticks","zigzag","wave","chevrons"}},
        {"dashLength","Trazo (px)","float",30.0,0.5,500},
        {"gapLength","Espacio (px)","float",20.0,0,500},
        {"dashSpeed","Velocidad trazos (px/s)","float",60.0,-1000,1000},
        {"dashPhase","Fase trazos (px)","float",0.0,-1000,1000},
        {"dashRoundCaps","Puntas de trazo redondas","bool",false},
        {"waveAmplitude","Amplitud onda (px)","float",10.0,0,200},
        {"waveFrequency","Ondas por perimetro","float",8.0,0.1,64},
        {"waveSpeed","Velocidad onda","float",1.0,-10,10},
        {"tickSpacing","Espacio marcas (px)","float",40.0,1,500},
        {"tickLength","Largo marcas (px)","float",20.0,0.5,200},
        {"animation","Animacion","enum","none",0,1,{"none","pulse","blink","strobe","chase","multiChase","breathing","sweep","trail","ripple","noiseFlow","random"}},
        {"speed","Velocidad","float",1.0,0,20},
        {"direction","Direccion","enum","cw",0,1,{"cw","ccw","pingPong","outwards","inwards"}},
        {"chaseCount","Cabezas chase","int",1,1,16},
        {"chaseLength","Longitud cabeza","float",0.12,0.001,1},
        {"chaseTail","Estela","float",0.25,0,1},
        {"chaseEasing","Curva chase","enum","linear",0,1,{"linear","easeInOut","bounce"}},
        {"pulseRate","Pulso (Hz)","float",1.0,0.01,20},
        {"pulseDepth","Profundidad pulso","float",0.5,0,1},
        {"pulseShape","Forma pulso","enum","sine",0,1,{"sine","triangle","square","saw","expDecay"}},
        {"strobeRate","Strobe (Hz)","float",8.0,0.1,30},
        {"strobeDuty","Tiempo encendido","float",0.5,0.01,1},
        {"phaseSpread","Desfase entre slices","float",0.0,0,1},
        {"phaseSpreadOrder","Orden desfase","enum","byIndex",0,1,{"byIndex","byX","byY","byDistanceFromCenter","random"}},
        {"seed","Semilla","int",1,0,1000000},
        {"noiseEnabled","Ruido","bool",false},
        {"noiseType","Tipo ruido","enum","perlin",0,1,{"perlin","simplex","value","worley"}},
        {"noiseAmount","Ruido posicion (px)","float",10.0,0,200},
        {"noiseScale","Escala ruido","float",0.02,0.0001,1},
        {"noiseSpeed","Velocidad ruido","float",1.0,0,20},
        {"noiseAffects","Ruido: pos/grosor/opacidad/color (bits)","int",1,0,15},
        {"glitchAmount","Glitch","float",0.0,0,1},
        {"glitchRate","Glitch (Hz)","float",8.0,0.1,60},
        {"jitter","Temblor (px)","float",0.0,0,100},
        {"flicker","Parpadeo neon","float",0.0,0,1},
        {"syncMode","Sincronizacion","enum","free",0,1,{"free","bpm","audio","osc"}},
        {"bpm","BPM del estilo","float",128.0,20,300},
        {"beatDivision","Beats por ciclo","enum","1",0,1,{"1/4","1/2","1","2","4","8"}},
        {"audioBand","Banda de audio","enum","bass",0,1,{"bass","lowMid","mid","high","all"}},
        {"audioAmount","Respuesta audio","float",1.0,0,5},
        {"audioSmoothing","Suavizado audio","float",0.7,0,0.999}
    };
    return p;
}
inline ofJson styleDefaults() {
    ofJson result=ofJson::object();
    for (const auto& p:parameterRegistry()) result[p.name]=p.initial;
    result["gradientStops"]=ofJson::array();
    result["presetName"]="Solid";
    return result;
}
inline ofJson resolveStyle(const ofJson& global,const ofJson& groups,const ofJson& binding) {
    auto result=styleDefaults(); result.update(global);
    auto group=binding.value("group",std::string{});
    if (!group.empty() && groups.contains(group)) result.update(groups.at(group));
    if (binding.contains("params")) result.update(binding["params"]);
    return result;
}
inline void validateStyle(const ofJson& values) {
    if (!values.is_object()) throw std::runtime_error("Parametros de estilo invalidos");
    for (const auto& p:parameterRegistry()) {
        if (!values.contains(p.name)) continue;
        const auto& v=values[p.name]; bool valid=true;
        if (p.type=="bool") valid=v.is_boolean();
        else if(p.type=="enum") valid=v.is_string() && std::find(p.options.begin(),p.options.end(),v.get<std::string>())!=p.options.end();
        else if(p.type=="color") { valid=v.is_array() && v.size()==4;
            if(valid) for(const auto& c:v) if(!c.is_number() || !std::isfinite(c.get<float>()) || c.get<float>()<0 || c.get<float>()>1) valid=false;
        } else valid=v.is_number() && std::isfinite(v.get<float>()) && v.get<float>()>=p.min && v.get<float>()<=p.max && (p.type!="int" || v.is_number_integer());
        if(!valid) throw std::runtime_error("Parametro invalido: " + p.name);
    }
    if(values.contains("gradientStops")) {
        const auto& stops=values["gradientStops"];
        if(!stops.is_array() || stops.size()>8) throw std::runtime_error("Se permiten hasta 8 paradas de gradiente");
        float previous=-1;
        for(const auto& stop:stops) {
            if(!stop.contains("pos") || !stop["pos"].is_number() || !stop.contains("color")) throw std::runtime_error("Parada de gradiente invalida");
            float pos=stop["pos"].get<float>();
            if(!std::isfinite(pos) || pos<previous || pos<0 || pos>1) throw std::runtime_error("Paradas deben estar ordenadas entre 0 y 1");
            validateStyle(ofJson{{"colorA",stop["color"]}}); previous=pos;
        }
    }
}
inline const Parameter* findParameter(const std::string& name){for(const auto& p:parameterRegistry())if(p.name==name)return &p;return nullptr;}
inline int colorComponent(const std::string& name){if(name.size()<3 || name[name.size()-2]!='.')return -1;auto pos=std::string("rgba").find(name.back());return pos==std::string::npos?-1:int(pos);}
inline std::vector<std::string> numericTargets(){std::vector<std::string> result;for(const auto& p:parameterRegistry()){if(p.type=="float"||p.type=="int")result.push_back(p.name);else if(p.type=="color")for(char c:std::string("rgba"))result.push_back(p.name+"."+c);}return result;}
inline float numericValue(const ofJson& style,const std::string& name){int c=colorComponent(name);if(c>=0)return style.at(name.substr(0,name.size()-2)).at(c).get<float>();return style.at(name).get<float>();}
inline void setNumeric(ofJson& target,const ofJson& effective,const std::string& name,float value){
    if(!std::isfinite(value))throw std::runtime_error("Modulacion no finita");
    int component=colorComponent(name);auto* meta=findParameter(component>=0?name.substr(0,name.size()-2):name);
    if(!meta || (component<0 && meta->type!="float" && meta->type!="int") || (component>=0 && meta->type!="color"))throw std::runtime_error("Parametro no numerico: "+name);
    if(component>=0){auto color=target.value(meta->name,effective.at(meta->name));color[component]=ofClamp(value,0.f,1.f);target[meta->name]=color;}
    else if(meta->type=="int")target[name]=int(std::round(ofClamp(value,meta->min,meta->max)));
    else target[name]=ofClamp(value,meta->min,meta->max);
}
}
