#include "ProjectIO.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
namespace sb {
void ProjectIO::save(const Project& p, const std::string& path) {
    namespace fs = std::filesystem;
    const auto target = fs::absolute(path);
    auto relative = fs::path(p.xmlPath).lexically_relative(target.parent_path());
    if (relative.empty()) throw std::runtime_error("XML y proyecto deben estar en la misma unidad");
    ofJson data = {{"schemaVersion",1}, {"xmlPath",relative.generic_string()},
        {"bindings",p.bindings}, {"screenEnabled",p.screenEnabled}, {"defaultStyle",p.defaultStyle},
        {"output",p.output}, {"osc",p.osc}};
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
    p.defaultStyle = data.at("defaultStyle"); p.output = data.at("output"); p.osc = data.at("osc");
    if (!p.bindings.is_object() || !p.screenEnabled.is_object() || !p.defaultStyle.is_object() ||
        !p.output.is_object() || !p.osc.is_object()) throw std::runtime_error("Proyecto mal formado");
    for (const auto& binding : p.bindings.items()) {
        const auto& b=binding.value();
        if (!b.is_object() || !b.contains("enabled") || !b["enabled"].is_boolean() ||
            !b.contains("styleName") || !b["styleName"].is_string() ||
            !b.contains("params") || !b["params"].is_object())
            throw std::runtime_error("Binding de slice mal formado");
    }
    for (const auto& state : p.screenEnabled.items())
        if (!state.value().is_boolean()) throw std::runtime_error("Estado de screen mal formado");
    return p;
}
}
