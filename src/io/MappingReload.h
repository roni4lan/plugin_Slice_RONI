#pragma once
#include "ResolumeXmlLoader.h"
#include <map>
namespace sb {
inline void reloadMapping(Composition& current,Project& project){
    auto next=ResolumeXmlLoader::load(project.xmlPath);
    auto bindings=project.bindings;auto routes=project.modulation;
    std::map<std::string,std::vector<std::string>> oldNames,newNames;
    for(const auto& screen:current.screens)for(const auto& slice:screen.slices)oldNames[slice.name].push_back(slice.id);
    for(const auto& screen:next.screens)for(const auto& slice:screen.slices)newNames[slice.name].push_back(slice.id);
    for(const auto& screen:next.screens)for(const auto& slice:screen.slices){
        if(bindings.contains(slice.id))continue;
        // Name fallback is safe only when the name is unique in both mappings.
        if(oldNames[slice.name].size()==1 && newNames[slice.name].size()==1){
            auto oldId=oldNames[slice.name].front();if(bindings.contains(oldId)){bindings[slice.id]=bindings[oldId];for(auto& route:routes)if(route.value("scope",std::string())=="slice" && route.value("bindingId",std::string())==oldId)route["bindingId"]=slice.id;}
        }
    }
    current=std::move(next);project.bindings=std::move(bindings);project.modulation=std::move(routes);
}
}
