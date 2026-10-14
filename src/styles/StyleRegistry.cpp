#include "StyleRegistry.h"
namespace sb {
StyleRegistry& StyleRegistry::instance(){static StyleRegistry registry;return registry;}
void StyleRegistry::add(std::shared_ptr<BorderStyle> style){registered[style->name()]=std::move(style);}
const BorderStyle& StyleRegistry::get(const std::string& name) const{
    auto it=registered.find(name);if(it==registered.end())throw std::runtime_error("Estilo no registrado: "+name);return *it->second;
}
void StyleRegistry::writeInitialPresets() const{
    ofDirectory::createDirectory(ofToDataPath("presets"),true,true);
    for(const auto& item:registered){auto path=ofToDataPath("presets/"+item.first+".json");if(!ofFile::doesFileExist(path))ofSavePrettyJson(path,{{"name",item.first},{"params",item.second->defaults()}});}
}
}
