#include "ofApp.h"
#include "io/ResolumeXmlLoader.h"
#include "io/ProjectIO.h"
#include <filesystem>
#include <fstream>
#include "pugixml.hpp"
#include "ui/Panels.h"
#include "model/StyleParams.h"
#include "geometry/Polyline.h"

void ofApp::setup() {
    ofSetFrameRate(60); ofSetEscapeQuitsApp(false); gui.setup();
    ofLogNotice("setup") << "Inicializando shaders";
    try { renderer.setup(); } catch(const std::exception& e){status=e.what();}
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
        selected.clear(); loaded=true; fitPending=true;
        renderer.clearCache();
        status="Mapping cargado: Arena " + composition.arenaVersion;
    } catch (const std::exception& e) { status=e.what(); ofLogError("open") << status; }
}
void ofApp::dialogOpen() {
    auto result=ofSystemLoadDialog("Abrir XML de Arena o proyecto JSON");
    if (result.bSuccess) open(result.getPath());
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
}
void ofApp::draw() {
    ofBackground(18,21,28);
    if(loaded){renderer.render(composition,project,ofGetElapsedTimef());if(!renderer.error.empty())status=renderer.error;}
    gui.begin();
    ImGui::SetNextWindowPos(ImVec2(0,0)); ImGui::SetNextWindowSize(ImVec2(ofGetWidth(),ofGetHeight()));
    ImGui::Begin("RONI RESO",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Abrir XML / proyecto","Ctrl+O")) dialogOpen();
            if (ImGui::MenuItem("Guardar proyecto","Ctrl+S",false,loaded)) saveProject();
            ImGui::EndMenu();
        }
        ImGui::Text("Pluguin Slice RONI RESO | %.1f FPS",ofGetFrameRate());
        ImGui::EndMenuBar();
    }
    ImGui::BeginChild("Controles",ImVec2(290,-35),true); sidebar(); ImGui::EndChild();
    ImGui::SameLine(); ImGui::BeginChild("Vista",ImVec2(0,-35),true); preview(); ImGui::EndChild();
    ImGui::TextWrapped("%s",status.c_str());
    ImGui::End(); gui.end();
}
void ofApp::sidebar() {
    if (ImGui::Button("Abrir XML / proyecto")) dialogOpen();
    if (!loaded) { ImGui::TextWrapped("Arrastra un XML de Advanced Output a esta ventana."); return; }
    ImGui::Text("Composicion: %d x %d",composition.width,composition.height);
    if(ImGui::Checkbox("Ver espacio de salida",&outputSpace))fitPending=true;
    ImGui::Checkbox("Ver bordes renderizados",&showBorders);
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
            if (ImGui::Selectable(slice.name.c_str(),selected==slice.id)) selected=slice.id;
            if (selected==slice.id) ImGui::TextWrapped("ID: %s",slice.id.c_str());
            ImGui::PopID();
        }
        ImGui::PopID(); ImGui::Separator();
    }
    ImGui::TextWrapped("Rueda: zoom\nBoton central: desplazar\nClick: seleccionar slice");
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
                if(!showBorders || selected==slice.id)draw->AddPolyline(vertices.data(),vertices.size(),selected==slice.id?IM_COL32(255,200,60,255):IM_COL32(40,220,245,255),ImDrawFlags_Closed,selected==slice.id?1:1.5f);
                draw->AddText(vertices[0],IM_COL32(240,240,245,255),slice.name.c_str());
                auto local=(mouse-pan)/zoom;
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hit.inside(local.x,local.y)) picked=slice.id;
            }
        }
        if (!picked.empty()) selected=picked;
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
    } catch(const std::exception& e) { report={{"passed",false},{"error",e.what()}}; }
    ofSavePrettyJson(ofToDataPath("self-test-report.json"),report);
    ofLogNotice("self-test") << report.dump();
    ofExit(report.value("passed",false)?0:1);
}
