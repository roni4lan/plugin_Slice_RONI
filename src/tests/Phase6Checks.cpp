#include "Phase6Checks.h"
#include "../io/MappingReload.h"
#include "../io/FileWatcher.h"
#include "../model/ProjectHistory.h"
#include "../model/StyleParams.h"
#include "pugixml.hpp"
#include <fstream>
namespace sb { namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
void write(const std::string& path,const std::string& text){std::ofstream stream(path,std::ios::binary);stream<<text;}
struct RestoreFile{
    std::string path,content;
    explicit RestoreFile(std::string path):path(std::move(path)),content(ofBufferFromFile(this->path).getText()){}
    ~RestoreFile(){write(path,content);}
};
}
ofJson phase6Checks(const Composition& fixture,const std::string& xmlPath,BorderRenderer& renderer){
    auto testPath=ofToDataPath("projects/reload-test.xml",true);ofFile::copyFromTo(xmlPath,testPath,false,true);
    Project project;project.xmlPath=testPath;project.defaultStyle=styleDefaults();auto mapping=fixture;
    auto id=fixture.screens[0].slices[0].id;
    project.bindings[id]={{"enabled",true},{"styleName","Neon"},{"params",{{"thickness",23.0}}}};
    FileWatcher watcher;watcher.reset(testPath);
    pugi::xml_document xml;require(xml.load_file(testPath.c_str()),"XML temporal no carga");
    auto size=xml.select_node("//CurrentCompositionTextureSize").node();
    require(size,"Tamano XML no encontrado");size.attribute("width").set_value(2560);size.attribute("height").set_value(1440);
    auto movedSlice=xml.select_node(("//Slice[@uniqueId='"+id+"']").c_str()).node();
    for(auto vertex:movedSlice.child("InputRect").children("v"))vertex.attribute("x").set_value(vertex.attribute("x").as_double()+100);
    require(xml.save_file(testPath.c_str()),"No se exporto XML de prueba");
    require(!watcher.poll(0) && !watcher.poll(0.2) && watcher.poll(0.5),"Watcher no espera exportacion estable");
    reloadMapping(mapping,project);
    require(mapping.width==2560 && mapping.height==1440 && project.bindings[id]["params"]["thickness"]==23.0 && project.bindings[id]["styleName"]=="Neon","Recarga pierde estilos o resolucion");
    require(std::abs(mapping.screens[0].slices[0].inputPoints[0].x-100)<0.01,"Recarga no actualiza geometria");
    renderer.render(mapping,project,0);ofPixels moved;renderer.frame().readToPixels(moved);require(moved.getColor(100,200).a>100,"Borde recargado no sigue nuevo contorno");
    auto slice=xml.select_node(("//*[@uniqueId='"+id+"']").c_str()).node();
    // The actual fixture uses id, not uniqueId; select the existing slice by either attribute.
    if(!slice)slice=xml.select_node(("//*[@id='"+id+"']").c_str()).node();
    require(slice,"ID de slice no encontrado");auto attr=slice.attribute("uniqueId");if(!attr)attr=slice.attribute("id");attr.set_value("9999999999999");xml.save_file(testPath.c_str());
    reloadMapping(mapping,project);require(project.bindings["9999999999999"]==project.bindings[id],"Respaldo por nombre pierde estilo");
    write(testPath,"<incomplete>");bool rejected=false;try{reloadMapping(mapping,project);}catch(...){rejected=true;}
    require(rejected && mapping.width==2560 && project.bindings["9999999999999"]["styleName"]=="Neon","XML invalido reemplaza mapping activo");
    ProjectHistory history;history.reset(project);project.defaultStyle["thickness"]=10.0;history.observe(project,true);project.defaultStyle["thickness"]=15.0;history.observe(project,false);
    require(history.undo(project) && project.defaultStyle["thickness"]==6.0,"Undo no agrupa gesto");require(history.redo(project) && project.defaultStyle["thickness"]==15.0,"Redo no restaura valor");
    renderer.render(fixture,project,0);ofPixels before;renderer.frame().readToPixels(before);
    auto frag=ofToDataPath("shaders/border_solid.frag",true);bool shaderRejected=false;
    {RestoreFile restore(frag);write(frag,"#version 330\ninvalid shader\n");shaderRejected=!renderer.reloadShaders();}
    require(shaderRejected,"Shader invalido aceptado");
    renderer.render(fixture,project,0);ofPixels after;renderer.frame().readToPixels(after);require(renderer.error.empty() && before.size()==after.size() && std::equal(before.begin(),before.end(),after.begin()),"Recarga fallida pierde imagen anterior");
    require(renderer.reloadShaders(),"Shaders restaurados no recargan");
    renderer.render(fixture,project,0);
    return {{"xmlReloadPreservesStyleAndResolution",true},{"reexportMovesRenderedBorder",true},{"uniqueNameFallback",true},{"invalidExportKeepsLastMapping",true},{"debouncedWatcher",true},{"undoRedoGesture",true},{"shaderReloadAtomic",true}};
}
}
