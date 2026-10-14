#include "ofApp.h"
#include "io/ResolumeXmlLoader.h"
#include "io/ProjectIO.h"
#include <filesystem>
#include <fstream>
#include "pugixml.hpp"
#include "ui/Panels.h"
#include "model/StyleParams.h"
#include "geometry/Polyline.h"
#include "styles/StyleRegistry.h"
#include <set>
#include "output/SpoutOutput.h"
#include "output/NdiOutput.h"
#include "ui/ControlPanels.h"
#include "modulation/LFO.h"
#include "tests/Phase5Checks.h"
#include "io/MappingReload.h"
#include "tests/Phase6Checks.h"

void ofApp::setup() {
    ofSetFrameRate(60); ofSetEscapeQuitsApp(false); gui.setup();
    sb::StyleRegistry::instance().writeInitialPresets();
    ofLogNotice("setup") << "Inicializando shaders";
    try { renderer.setup(); } catch(const std::exception& e){status=e.what();}
    shaderWatcher.reset(ofToDataPath("shaders",true));
    if (!initialPath.empty()) open(initialPath);
    if (testMode) selfTest();
}
void ofApp::open(const std::string& path) {
    try {
        sb::Project next;
        bool json = ofToLower(ofFilePath::getFileExt(path)) == "json";
        if (json) next=sb::ProjectIO::load(path);
        else next.xmlPath=std::filesystem::absolute(path).string();
        auto mapping=sb::ResolumeXmlLoader::load(next.xmlPath);
        // Commit only after the entire file validates, preserving the current project on error.
        project=std::move(next); composition=std::move(mapping);
        projectPath=json ? std::filesystem::absolute(path).string() : "";
        selected.clear();selection.clear(); loaded=true; fitPending=true;
        mappingWatcher.reset(project.xmlPath);history.reset(project);modulation=sb::ModulationMatrix{};
        renderer.clearCache();
        output.close();
        audio.stop();osc.stop();audioSignature.clear();beatClock=sb::BeatClock{};beatClock.setBeats(0,ofGetElapsedTimef());
        status="Mapping cargado: Arena " + composition.arenaVersion;
    } catch (const std::exception& e) { status=e.what(); ofLogError("open") << status; }
}
void ofApp::dialogOpen() {
    auto result=ofSystemLoadDialog("Abrir XML de Arena o proyecto JSON");
    if (result.bSuccess) open(result.getPath());
}
void ofApp::update(){
    if(!loaded)return;double time=ofGetElapsedTimef();
    if(!testMode && time>=nextWatch){nextWatch=time+0.15;if(mappingWatcher.poll(time))reloadMapping();if(shaderWatcher.poll(time)){status=renderer.reloadShaders()?"Shaders recargados":renderer.error;}}
    beatClock.setBpm(project.clock.value("bpm",128.0),time);beatClock.setPlaying(project.clock.value("playing",true),time);
    osc.update(project,composition,beatClock,modulation,time);
    auto key=project.audio.dump();if(key!=audioSignature){audioSignature=key;if(project.audio.value("enabled",false))audio.start(project.audio.value("deviceIndex",0));else audio.stop();}
}
void ofApp::reloadMapping(){
    try{sb::reloadMapping(composition,project);renderer.clearCache();fitPending=true;status="Mapping actualizado; estilos conservados";history.observe(project);}
    catch(const std::exception& e){status=std::string("Recarga rechazada; mapping anterior activo: ")+e.what();}
}
void ofApp::select(const std::string& id,bool multiple){
    if(!multiple)selection.clear();
    if(multiple && selection.count(id))selection.erase(id);else selection.insert(id);
    selected=selection.count(id)?id:selection.empty()?"":*selection.begin();
}
void ofApp::saveProject() {
    if (!loaded) return;
    auto result=ofSystemSaveDialog(projectPath.empty() ? "RONI_RESO.json" : projectPath,"Guardar proyecto");
    if (!result.bSuccess) return;
    try { sb::ProjectIO::save(project,result.getPath()); projectPath=result.getPath(); status="Proyecto guardado"; }
    catch (const std::exception& e) { status=e.what(); }
}
void ofApp::dragEvent(ofDragInfo info) { if (!info.files.empty()) open(info.files.front().string()); }
void ofApp::keyPressed(int key) {
    if (ImGui::GetCurrentContext() && ImGui::GetIO().WantTextInput) return;
    if (key=='o' && ofGetKeyPressed(OF_KEY_CONTROL)) dialogOpen();
    if (key=='s' && ofGetKeyPressed(OF_KEY_CONTROL)) saveProject();
    if (key=='f') fitPending=true;
    if(key==OF_KEY_F12)performance=!performance;
    if(key=='r' && loaded)reloadMapping();
    if(key=='z' && ofGetKeyPressed(OF_KEY_CONTROL)){bool ok=ofGetKeyPressed(OF_KEY_SHIFT)?history.redo(project):history.undo(project);if(ok){renderer.clearCache();status="Proyecto restaurado";}}
    if(key=='y' && ofGetKeyPressed(OF_KEY_CONTROL) && history.redo(project)){renderer.clearCache();status="Proyecto restaurado";}
    if(key=='t'){auto bpm=beatClock.tap(ofGetElapsedTimef());project.clock["bpm"]=bpm;project.defaultStyle["bpm"]=bpm;}
}
void ofApp::draw() {
    ofBackground(18,21,28);
    if(loaded){try{ofSetFrameRate(std::max(60,project.output.value("fps",60)));auto animated=modulation.apply(project,composition,ofGetElapsedTimef(),audio.bands(),beatClock,osc.values);renderer.render(composition,animated,ofGetElapsedTimef());if(!renderer.error.empty())status=renderer.error;else if(!lastRenderError.empty())status="Mapping cargado: Arena "+composition.arenaVersion;lastRenderError=renderer.error;output.update(project.output,renderer.frame(),renderer.preview());}catch(const std::exception& e){status=e.what();output.close();}}
    if(performance){ofSetColor(255);if(loaded && renderer.preview().isAllocated()){float scale=std::min(float(ofGetWidth())/composition.width,float(ofGetHeight())/composition.height);renderer.preview().draw((ofGetWidth()-composition.width*scale)/2,(ofGetHeight()-composition.height*scale)/2,composition.width*scale,composition.height*scale);}ofDrawBitmapStringHighlight("Performance | F12: controles | "+output.status,12,20);history.observe(project);return;}
    gui.begin();
    ImGui::SetNextWindowPos(ImVec2(0,0)); ImGui::SetNextWindowSize(ImVec2(ofGetWidth(),ofGetHeight()));
    ImGui::Begin("RONI RESO",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Abrir XML / proyecto","Ctrl+O")) dialogOpen();
            if (ImGui::MenuItem("Guardar proyecto","Ctrl+S",false,loaded)) saveProject();
            if(ImGui::MenuItem("Recargar mapping","R",false,loaded))reloadMapping();
            if(ImGui::MenuItem("Deshacer","Ctrl+Z",false,loaded))history.undo(project);
            if(ImGui::MenuItem("Rehacer","Ctrl+Y",false,loaded))history.redo(project);
            if(ImGui::MenuItem("Performance","F12"))performance=true;
            ImGui::EndMenu();
        }
        ImGui::Text("Pluguin Slice RONI RESO | %.1f FPS",ofGetFrameRate());
        ImGui::EndMenuBar();
    }
    ImGui::BeginChild("Controles",ImVec2(290,-35),true); sidebar(); ImGui::EndChild();
    ImGui::SameLine(); ImGui::BeginChild("Vista",ImVec2(0,-35),true); preview(); ImGui::EndChild();
    ImGui::TextWrapped("%s",status.c_str());
    ImGui::End(); gui.end();
    if(loaded)history.observe(project,ImGui::IsMouseDown(0) || ImGui::IsAnyItemActive());
}
void ofApp::sidebar() {
    if (ImGui::Button("Abrir XML / proyecto")) dialogOpen();
    if (!loaded) { ImGui::TextWrapped("Arrastra un XML de Advanced Output a esta ventana."); return; }
    ImGui::Text("Composicion: %d x %d",composition.width,composition.height);
    if(ImGui::CollapsingHeader("Salida a Resolume",ImGuiTreeNodeFlags_DefaultOpen)){
        bool enabled=project.output.value("enabled",false);if(ImGui::Checkbox("Enviar textura",&enabled))project.output["enabled"]=enabled;
        auto type=project.output.value("type",std::string("Spout"));
        if(ImGui::BeginCombo("Salida",type.c_str())){for(auto candidate:{"Spout","NDI","Syphon"})if(ImGui::Selectable(candidate,type==candidate))project.output["type"]=candidate;ImGui::EndCombo();}
        char name[201];auto current=project.output.value("name",std::string("RONI RESO"));std::snprintf(name,sizeof(name),"%s",current.c_str());
        if(ImGui::InputText("Emisor",name,sizeof(name)))project.output["name"]=std::string(name);
        int fps=project.output.value("fps",60);if(ImGui::SliderInt("FPS salida",&fps,1,120))project.output["fps"]=fps;
        bool custom=project.output.value("resolutionMode",std::string("composition"))=="custom";
        if(ImGui::Checkbox("Resolucion personalizada",&custom))project.output["resolutionMode"]=custom?"custom":"composition";
        if(custom){int w=project.output.value("width",composition.width),h=project.output.value("height",composition.height);if(ImGui::InputInt("Ancho",&w))project.output["width"]=ofClamp(w,1,16384);if(ImGui::InputInt("Alto",&h))project.output["height"]=ofClamp(h,1,16384);}
        else ImGui::Text("Salida: %d x %d",composition.width,composition.height);
        auto alpha=project.output.value("alphaMode",std::string("premultiplied"));
        if(type=="NDI")ImGui::Text("Alfa NDI: directo (RGBA)");
        else if(ImGui::BeginCombo("Alfa",alpha.c_str())){for(auto option:{"premultiplied","straight"})if(ImGui::Selectable(option,alpha==option))project.output["alphaMode"]=option;ImGui::EndCombo();}
        ImGui::TextWrapped("%s",output.status.c_str());ImGui::Text("Enviados: %llu | %.1f FPS",static_cast<unsigned long long>(output.sentFrames),output.measuredFps);
        if(ImGui::Button("Reintentar salida"))output.close();
    }
    if(ImGui::Checkbox("Ver espacio de salida",&outputSpace))fitPending=true;
    ImGui::Checkbox("Ver bordes renderizados",&showBorders);
    sb::drawControlPanels(project,composition,selected,audio,beatClock,modulation,osc);
    if (ImGui::Button("Ajustar vista (F)")) fitPending=true;
    ImGui::Separator();
    for (auto& screen : composition.screens) {
        ImGui::PushID(screen.id.c_str());
        bool enabled=project.screenEnabled.value(screen.id,screen.enabled);
        if (ImGui::Checkbox(screen.name.c_str(),&enabled)) project.screenEnabled[screen.id]=enabled;
        ImGui::TextDisabled("Salida: %d x %d | Hidden: %d",screen.width,screen.height,screen.hidden);
        for (auto& slice : screen.slices) {
            ImGui::PushID(slice.id.c_str());
            auto& binding=project.bindings[slice.id];
            if (binding.is_null()) binding={{"enabled",slice.enabled},{"styleName","Solid"},{"params",ofJson::object()}};
            bool visible=binding.value("enabled",slice.enabled);
            if (ImGui::Checkbox("##enabled",&visible)) binding["enabled"]=visible;
            ImGui::SameLine();
            if (ImGui::Selectable(slice.name.c_str(),selection.count(slice.id)>0)) select(slice.id,ImGui::GetIO().KeyCtrl);
            if (selected==slice.id) ImGui::TextWrapped("ID: %s",slice.id.c_str());
            ImGui::PopID();
        }
        ImGui::PopID(); ImGui::Separator();
    }
    ImGui::TextWrapped("Rueda: zoom\nBoton central: desplazar\nClick: seleccionar slice");
    if(selection.size()>1){
        ImGui::Text("%d slices seleccionados",int(selection.size()));
        if(ImGui::Button("Habilitar todos"))for(const auto& id:selection)project.bindings[id]["enabled"]=true;
        ImGui::SameLine();if(ImGui::Button("Ocultar todos"))for(const auto& id:selection)project.bindings[id]["enabled"]=false;
        if(ImGui::BeginCombo("Estilo para seleccion","Elegir")){for(const auto& entry:sb::StyleRegistry::instance().styles())if(ImGui::Selectable(entry.first.c_str()))for(const auto& id:selection){project.bindings[id]["params"]=entry.second->defaults();project.bindings[id]["styleName"]=entry.first;}ImGui::EndCombo();}
        if(ImGui::BeginCombo("Grupo para seleccion","Elegir")){if(ImGui::Selectable("Sin grupo"))for(const auto& id:selection)project.bindings[id]["group"]="";for(const auto& item:project.groups.items())if(ImGui::Selectable(item.key().c_str()))for(const auto& id:selection)project.bindings[id]["group"]=item.key();ImGui::EndCombo();}
    }
    sb::drawStylePanel(project,selected);
}
void ofApp::preview() {
    ImVec2 origin=ImGui::GetCursorScreenPos(), area=ImGui::GetContentRegionAvail();
    if (area.x < 1 || area.y < 1) return;
    ImGui::InvisibleButton("canvas",area,ImGuiButtonFlags_MouseButtonLeft|ImGuiButtonFlags_MouseButtonMiddle);
    bool hovered=ImGui::IsItemHovered(); auto& io=ImGui::GetIO();
    int width=composition.width, height=composition.height;
    if (outputSpace && !composition.screens.empty()) { width=composition.screens[0].width; height=composition.screens[0].height; }
    if (fitPending && loaded) { zoom=std::min((area.x-50)/width,(area.y-50)/height); zoom=std::max(zoom,0.001f);
        pan={(area.x-width*zoom)/2,(area.y-height*zoom)/2}; fitPending=false; }
    glm::vec2 mouse{io.MousePos.x-origin.x,io.MousePos.y-origin.y};
    if (hovered && loaded && io.MouseWheel!=0) {
        float next=ofClamp(zoom*std::pow(1.15f,io.MouseWheel),0.001f,100.f);
        pan=mouse-(mouse-pan)*(next/zoom); zoom=next;
    }
    if (hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) pan+=glm::vec2(io.MouseDelta.x,io.MouseDelta.y);
    auto* draw=ImGui::GetWindowDrawList();
    draw->PushClipRect(origin,ImVec2(origin.x+area.x,origin.y+area.y),true);
    auto transform=[&](glm::vec2 p){ p=pan+p*zoom; return ImVec2(origin.x+p.x,origin.y+p.y); };
    if (loaded) {
        draw->AddRectFilled(transform({0,0}),transform({width,height}),IM_COL32(29,34,44,255));
        draw->AddRect(transform({0,0}),transform({width,height}),IM_COL32(110,120,140,255));
        if(showBorders && renderer.preview().isAllocated()) {
            // The UI texture uses straight alpha; the output keeps premultiplied alpha.
            draw->AddImage((ImTextureID)(uintptr_t)renderer.preview().getTexture().getTextureData().textureID,
                transform({0,0}),transform({composition.width,composition.height}),ImVec2(0,0),ImVec2(1,1));
        }
        std::string picked;
        for (const auto& screen:composition.screens) {
            if (!project.screenEnabled.value(screen.id,screen.enabled)) continue;
            for (const auto& slice:screen.slices) {
                if (!project.bindings[slice.id].value("enabled",slice.enabled)) continue;
                const auto& contour=outputSpace?slice.outputPoints:slice.inputPoints;
                std::vector<ImVec2> vertices; ofPolyline hit;
                for (auto p:contour) { vertices.push_back(transform(p)); hit.addVertex(p.x,p.y); }
                hit.close();
                if(!showBorders || selection.count(slice.id))draw->AddPolyline(vertices.data(),vertices.size(),selection.count(slice.id)?IM_COL32(255,200,60,255):IM_COL32(40,220,245,255),ImDrawFlags_Closed,1.5f);
                draw->AddText(vertices[0],IM_COL32(240,240,245,255),slice.name.c_str());
                auto local=(mouse-pan)/zoom;
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hit.inside(local.x,local.y)) picked=slice.id;
            }
        }
        if (!picked.empty()) select(picked,io.KeyCtrl);
    } else draw->AddText(ImVec2(origin.x+30,origin.y+30),IM_COL32(170,180,195,255),"Carga tu mapping para ver los slices");
    draw->PopClipRect();
}
void ofApp::selfTest() {
    ofJson report;
    ofSavePrettyJson(ofToDataPath("self-test-report.json"),{{"passed",false},{"state","running"}});
    ofLogNotice("self-test") << "Carga y persistencia";
    try {
        if (!loaded) throw std::runtime_error("El XML inicial no cargo");
        if (composition.width!=1920 || composition.height!=1080 || composition.screens.size()!=1 || composition.screens[0].slices.size()!=3)
            throw std::runtime_error("El ejemplo real difiere del esperado");
        auto& first=composition.screens[0].slices[0];
        if (first.name!="Slice 3" || first.id!="1791941903616") throw std::runtime_error("Identidad incorrecta");
        project.bindings[first.id]={{"enabled",false},{"styleName","Solid"},{"params",{{"thickness",17.0}}}};
        project.output["name"]="RoundTrip"; project.osc["port"]=8123;
        ofDirectory::createDirectory(ofToDataPath("projects"),true,true);
        auto target=ofToDataPath("projects/self-test.json",true);
        sb::ProjectIO::save(project,target); auto restored=sb::ProjectIO::load(target);
        auto mapped=sb::ResolumeXmlLoader::load(restored.xmlPath);
        if (restored.bindings!=project.bindings || restored.output!=project.output || restored.osc!=project.osc || mapped.width!=composition.width)
            throw std::runtime_error("El proyecto no restauro ajustes");
        pugi::xml_document fixture;
        if (!fixture.load_file(project.xmlPath.c_str())) throw std::runtime_error("Fixture invalido");
        auto size=fixture.child("XmlState").child("ScreenSetup").child("CurrentCompositionTextureSize");
        size.attribute("width").set_value(2560); size.attribute("height").set_value(1440);
        auto alternate=ofToDataPath("projects/alternate-size.xml",true);
        if (!fixture.save_file(alternate.c_str())) throw std::runtime_error("No se pudo escribir fixture");
        auto resized=sb::ResolumeXmlLoader::load(alternate);
        if (resized.width!=2560 || resized.height!=1440 || resized.screens[0].width!=1920)
            throw std::runtime_error("La resolucion de composicion no es independiente de la salida");
        auto firstPoint=fixture.child("XmlState").child("ScreenSetup").child("screens").child("Screen").child("layers").child("Slice").child("InputRect").child("v");
        firstPoint.attribute("x").set_value("not-a-number");
        fixture.save_file(alternate.c_str());
        bool rejected=false;
        try { sb::ResolumeXmlLoader::load(alternate); } catch(const std::exception&) { rejected=true; }
        if (!rejected) throw std::runtime_error("Coordenada invalida no rechazada");
        report={{"passed",true},{"screens",1},{"slices",3},{"projectRoundTrip",true},
                {"variableResolution",true},{"invalidCoordinateRejected",true}};
        ofLogNotice("self-test") << "Geometria";
        auto style=sb::styleDefaults();style["thickness"]=16.0;
        auto rectangle=std::vector<glm::vec2>{{100,100},{500,100},{500,400},{100,400}};
        if(std::abs(sb::Polyline::length(rectangle)-1400)>0.01f || sb::Polyline::resample(rectangle,10).size()!=140)
            throw std::runtime_error("Longitud/remuestreo incorrectos");
        for(auto join:{"miter","round","bevel"}) {
            style["joinStyle"]=join;auto mesh=sb::BorderMesh::build(rectangle,style);
            if(mesh.mesh.getNumVertices()==0)throw std::runtime_error("Mesh vacia");
            for(auto v:mesh.mesh.getVertices())if(!std::isfinite(v.x)||!std::isfinite(v.y))throw std::runtime_error("Vertice invalido");
        }
        sb::Composition visual;visual.width=1920;visual.height=1080;
        sb::Screen screen;screen.id="test-screen";screen.name="Test";screen.width=visual.width;screen.height=visual.height;
        sb::Slice slice;slice.id="test-slice";slice.name="Test";slice.inputPoints=rectangle;slice.outputPoints=rectangle;screen.slices.push_back(slice);visual.screens.push_back(screen);
        sb::Project renderProject;renderProject.defaultStyle=sb::styleDefaults();renderProject.defaultStyle["thickness"]=16.0;
        ofLogNotice("self-test") << "Render y transparencia";
        renderer.render(visual,renderProject,0);ofPixels pixels;renderer.frame().readToPixels(pixels);
        if(!renderer.error.empty())throw std::runtime_error(renderer.error);
        if(!pixels.isAllocated() || pixels.getNumChannels()!=4)throw std::runtime_error("Readback RGBA invalido");
        if(pixels.getColor(300,250).a!=0 || pixels.getColor(700,100).a!=0)
            throw std::runtime_error("El fondo/relleno no es transparente");
        // oF's FBO projection makes texture row zero correspond to input y=0.
        if(pixels.getColor(100,250).a<240 || pixels.getColor(100,1080-1-250).a!=0)
            throw std::runtime_error("Borde no renderizado en posicion esperada");
        renderProject.defaultStyle["opacity"]=0.5;
        renderer.render(visual,renderProject,0);renderer.frame().readToPixels(pixels);
        auto pixel=pixels.getColor(100,250);
        if(pixel.a<120 || pixel.a>135 || pixel.b>pixel.a+1)throw std::runtime_error("Alpha no premultiplicado correctamente");
        renderProject.defaultStyle["colorA"]={1.0,0.0,0.0,1.0};
        auto overlay=slice;overlay.id="overlay";visual.screens[0].slices.push_back(overlay);
        renderProject.bindings[overlay.id]={{"enabled",true},{"params",{{"colorA",{0.0,0.0,1.0,1.0}}}}};
        for(auto blend:{"alpha","screen","multiply"}) {
            renderProject.defaultStyle["blendMode"]=blend;renderer.render(visual,renderProject,0);renderer.frame().readToPixels(pixels);
            auto p=pixels.getColor(100,250);int expectedRed=std::string(blend)=="screen"?128:64;
            int expectedBlue=std::string(blend)=="multiply"?64:128;
            if(std::abs(int(p.r)-expectedRed)>3 || std::abs(int(p.b)-expectedBlue)>3 || std::abs(int(p.a)-191)>3)
                throw std::runtime_error(std::string("Mezcla/alfa incorrectos: ")+blend+" RGB="+ofToString(p.r)+","+ofToString(p.b)+" A="+ofToString(p.a));
        }
        renderProject.defaultStyle=sb::styleDefaults();renderProject.defaultStyle["thickness"]=16.0;renderProject.bindings.clear();
        visual.screens[0].slices.clear();
        for(int i=0;i<50;++i){auto clone=slice;clone.id="perf-"+ofToString(i);float x=(i%10)*190+20,y=(i/10)*210+20;clone.inputPoints={{x,y},{x+150,y},{x+150,y+170},{x,y+170}};visual.screens[0].slices.push_back(clone);}
        ofLogNotice("self-test") << "Rendimiento con 50 slices";
        renderer.clearCache();renderer.render(visual,renderProject,0);glFinish();auto before=renderer.regenerations;
        double start=ofGetElapsedTimef();
        for(int frame=0;frame<120;++frame){renderer.render(visual,renderProject,frame/60.f);glFinish();}
        double fps=120/(ofGetElapsedTimef()-start);
        if(renderer.regenerations!=before)throw std::runtime_error("La mesh se regenera sin cambios geometricos");
        if(fps<60)throw std::runtime_error("Rendimiento inferior a 60 FPS con 50 slices: "+ofToString(fps));
        sb::Project inheritance;inheritance.defaultStyle["thickness"]=8.0;
        inheritance.groups["grupo"]={{"thickness",12.0}};
        auto bound=ofJson{{"group","grupo"},{"params",{{"thickness",17.0}}}};
        if(sb::resolveStyle(inheritance.defaultStyle,inheritance.groups,bound)["thickness"]!=17.0)throw std::runtime_error("Herencia de slice incorrecta");
        bound["params"].erase("thickness");
        if(sb::resolveStyle(inheritance.defaultStyle,inheritance.groups,bound)["thickness"]!=12.0)throw std::runtime_error("Herencia de grupo incorrecta");
        bound.erase("group");
        if(sb::resolveStyle(inheritance.defaultStyle,inheritance.groups,bound)["thickness"]!=8.0)throw std::runtime_error("Herencia global incorrecta");
        report["geometryAndJoins"]=true;report["transparentBackground"]=true;report["premultipliedAlpha"]=true;
        report["meshCache"]=true;report["benchmark50SlicesFps"]=fps;report["inheritance"]=true;report["alphaAwareBlendModes"]=true;
        renderer.frame().readToPixels(pixels);ofSaveImage(pixels,ofToDataPath("phase2-check.png"));
        ofLogNotice("self-test") << "Ocho estilos y animacion";
        auto imageHash=[](const ofPixels& p){uint64_t hash=1469598103934665603ull;for(size_t i=0;i<p.size();++i){hash^=p.getData()[i];hash*=1099511628211ull;}return hash;};
        sb::Composition sample=visual;sample.width=640;sample.height=360;sample.screens[0].slices={slice};
        sample.screens[0].slices[0].inputPoints={{100,100},{540,100},{540,260},{100,260}};
        std::set<uint64_t> signatures;int animatedStyles=0;
        for(const auto& preset:sb::StyleRegistry::instance().styles()){
            renderProject.defaultStyle=sb::styleDefaults();renderProject.defaultStyle.update(preset.second->defaults());renderProject.defaultStyle["thickness"]=12.0;
            renderer.render(sample,renderProject,0.123f);renderer.frame().readToPixels(pixels);
            if(!renderer.error.empty())throw std::runtime_error(renderer.error);
            auto firstHash=imageHash(pixels);signatures.insert(firstHash);
            ofSaveImage(pixels,ofToDataPath("check-"+preset.first+".png"));
            renderer.render(sample,renderProject,0.487f);renderer.frame().readToPixels(pixels);
            if(imageHash(pixels)!=firstHash)++animatedStyles;
        }
        if(signatures.size()<8 || animatedStyles<6)throw std::runtime_error("Los estilos no son distintos/animados");
        renderProject.defaultStyle=sb::styleDefaults();renderProject.defaultStyle.update(sb::StyleRegistry::instance().get("Neon").defaults());renderProject.defaultStyle["thickness"]=12.0;
        renderProject.defaultStyle["glowMode"]="bloom";renderer.render(sample,renderProject,0);renderer.frame().readToPixels(pixels);
        if(pixels.getColor(100,80).a==0 || pixels.getColor(20,20).a!=0)throw std::runtime_error("Blur bloom no produce halo transparente correcto");
        ofSaveImage(pixels,ofToDataPath("check-Bloom.png"));
        renderProject.defaultStyle["glowMode"]="falloff";renderer.render(visual,renderProject,0);glFinish();before=renderer.regenerations;start=ofGetElapsedTimef();
        for(int frame=0;frame<120;++frame){renderer.render(visual,renderProject,frame/60.f);glFinish();}
        double glowFps=120/(ofGetElapsedTimef()-start);
        if(glowFps<60 || before!=renderer.regenerations)throw std::runtime_error("Rendimiento/cache con halo no cumple");
        report["distinctStyles"]=signatures.size();report["animatedStyles"]=animatedStyles;report["bloomHalo"]=true;report["benchmark50SlicesFalloffFps"]=glowFps;
        ofLogNotice("self-test") << "Spout y NDI: round-trip de textura y alfa";
        sample.screens[0].slices[0].inputPoints={{60,30},{500,30},{500,160},{60,160}};
        renderProject.defaultStyle=sb::styleDefaults();renderProject.defaultStyle["opacity"]=0.5;renderProject.defaultStyle["thickness"]=14.0;
        renderer.render(sample,renderProject,0);glFinish();
        sb::SpoutOutput spout;spout.setup("SliceBorders-SelfTest-Spout",60);
        if(!spout.verifyLoopback(renderer.frame().getTexture()))throw std::runtime_error("Spout no conserva textura/alfa/orientacion");spout.close();
        ofLogNotice("self-test") << "Spout completado; inicializando NDI";
        sb::NdiOutput ndi;ndi.setup("SliceBorders-SelfTest-NDI",60);
        if(!ndi.verifyLoopback(renderer.preview().getTexture()))throw std::runtime_error("NDI no conserva resolucion/alfa/orientacion");ndi.close();
        report["spoutTextureRoundTrip"]=true;report["ndiAlphaRoundTrip"]=true;
        ofLogNotice("self-test") << "Modulacion, audio, BPM y OSC UDP";
        report["phase5"]=sb::phase5Checks(composition,project.xmlPath,renderer);
        ofLogNotice("self-test") << "Recarga XML/shaders y deshacer/rehacer";
        report["phase6"]=sb::phase6Checks(composition,project.xmlPath,renderer);
    } catch(const std::exception& e) { report={{"passed",false},{"error",e.what()}}; }
    ofSavePrettyJson(ofToDataPath("self-test-report.json"),report);
    ofLogNotice("self-test") << report.dump();
    ofExit(report.value("passed",false)?0:1);
}
