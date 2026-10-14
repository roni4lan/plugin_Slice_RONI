#include "ProjectIO.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "../model/StyleParams.h"
namespace sb {
void ProjectIO::save(const Project& p, const std::string& path) {
    namespace fs = std::filesystem;
    const auto target = fs::absolute(path);
    auto relative = fs::path(p.xmlPath).lexically_relative(target.parent_path());
    if (relative.empty()) throw std::runtime_error("XML y proyecto deben estar en la misma unidad");
    ofJson data = {{"schemaVersion",1}, {"xmlPath",relative.generic_string()},
        {"bindings",p.bindings}, {"groups",p.groups}, {"screenEnabled",p.screenEnabled}, {"defaultStyle",p.defaultStyle},
        {"output",p.output}, {"osc",p.osc}, {"clock",p.clock}, {"audio",p.audio}, {"modulation",p.modulation}};
    // A sibling temporary file prevents a failed write from truncating the project.
    const auto temp = fs::path(target.string()+".tmp");
    { std::ofstream file(temp); file << data.dump(2) << '\n'; file.flush();
      if (!file) throw std::runtime_error("No se pudo guardar el proyecto"); }
    // ofFile handles replacement on Windows, where rename does not replace a target.
    if (!ofFile::moveFromTo(temp.string(),target.string(),false,true))
        throw std::runtime_error("No se pudo reemplazar el proyecto");
}
Project ProjectIO::load(const std::string& path) {
    std::ifstream file(path); if (!file) throw std::runtime_error("No se pudo abrir el proyecto");
    ofJson data; file >> data;
    if (data.at("schemaVersion") != 1) throw std::runtime_error("Version de proyecto no soportada");
    Project p;
    auto xml = std::filesystem::path(data.at("xmlPath").get<std::string>());
    p.xmlPath = (std::filesystem::absolute(path).parent_path()/xml).lexically_normal().string();
    p.bindings = data.at("bindings"); p.screenEnabled = data.at("screenEnabled");
    p.groups = data.value("groups",ofJson::object());
    p.defaultStyle = data.at("defaultStyle"); p.output = data.at("output"); p.osc = data.at("osc");
    p.clock=data.value("clock",p.clock);p.audio=data.value("audio",p.audio);p.modulation=data.value("modulation",ofJson::array());
    if (!p.bindings.is_object() || !p.screenEnabled.is_object() || !p.defaultStyle.is_object() ||
        !p.output.is_object() || !p.osc.is_object() || !p.groups.is_object()) throw std::runtime_error("Proyecto mal formado");
    for (const auto& binding : p.bindings.items()) {
        const auto& b=binding.value();
        if (!b.is_object() || !b.contains("enabled") || !b["enabled"].is_boolean() ||
            !b.contains("styleName") || !b["styleName"].is_string() ||
            !b.contains("params") || !b["params"].is_object())
            throw std::runtime_error("Binding de slice mal formado");
        validateStyle(b["params"]);
    }
    for (const auto& state : p.screenEnabled.items())
        if (!state.value().is_boolean()) throw std::runtime_error("Estado de screen mal formado");
    validateStyle(p.defaultStyle);
    for(const auto& group:p.groups.items())validateStyle(group.value());
    for(auto* settings:{&p.output,&p.osc,&p.audio})if(settings->contains("enabled") && !(*settings)["enabled"].is_boolean())throw std::runtime_error("Enabled debe ser booleano");
    if(p.output.contains("fps") && (!p.output["fps"].is_number_integer() || p.output["fps"].get<int>()<1 || p.output["fps"].get<int>()>120))throw std::runtime_error("FPS de salida invalido");
    if(p.osc.contains("port") && (!p.osc["port"].is_number_integer() || p.osc["port"].get<int>()<1 || p.osc["port"].get<int>()>65535))throw std::runtime_error("Puerto OSC invalido");
    if(p.audio.contains("deviceIndex") && (!p.audio["deviceIndex"].is_number_integer() || p.audio["deviceIndex"].get<int>()<0))throw std::runtime_error("Indice de audio invalido");
    if(!p.modulation.is_array() || p.modulation.size()>128 || !p.clock.is_object() || !p.audio.is_object())throw std::runtime_error("Configuracion de modulacion invalida");
    if(!p.clock.value("bpm",ofJson(128.0)).is_number() || p.clock.value("bpm",128.f)<20 || p.clock.value("bpm",128.f)>300 || !std::isfinite(p.clock.value("bpm",128.f)) || !p.clock.value("playing",ofJson(true)).is_boolean())throw std::runtime_error("Reloj invalido");
    for(const auto& route:p.modulation){
        if(!route.is_object() || !route.contains("id") || !route["id"].is_string() || !route.contains("target") || !route["target"].is_string())throw std::runtime_error("Ruta de modulacion invalida");
        auto targets=numericTargets();auto target=route["target"].get<std::string>();if(std::find(targets.begin(),targets.end(),target)==targets.end())throw std::runtime_error("Destino numerico desconocido");
        for(auto field:{"source","scope","bindingId","shape","audioBand","sourceKey","curve","envelopeMode"})if(route.contains(field) && !route[field].is_string())throw std::runtime_error("Texto de ruta invalido");
        for(auto field:{"enabled","triggerOnBeat"})if(route.contains(field) && !route[field].is_boolean())throw std::runtime_error("Booleano de ruta invalido");
        for(auto field:{"frequency","amount","offset","phase","smoothing","attack","decay","sustain","release"})if(route.contains(field) && (!route[field].is_number() || !std::isfinite(route[field].get<float>())))throw std::runtime_error("Valor de ruta invalido");
    }
    return p;
}
}
