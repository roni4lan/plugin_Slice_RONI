#include "BorderRenderer.h"
#include "../model/StyleParams.h"
namespace sb { namespace {
glm::vec4 color(const ofJson& value){return {value[0].get<float>(),value[1].get<float>(),value[2].get<float>(),value[3].get<float>()};}
}
void BorderRenderer::setup(){
    ofDisableArbTex();
    if(!shader.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/border_solid.frag")) ||
       !composite.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/composite.frag")) ||
       !previewShader.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/preview.frag")))
        throw std::runtime_error("No se pudieron cargar los shaders");
}
void BorderRenderer::allocate(int w,int h){
    if(target.isAllocated() && target.getWidth()==w && target.getHeight()==h)return;
    ofFbo::Settings settings;settings.width=w;settings.height=h;settings.internalformat=GL_RGBA8;settings.textureTarget=GL_TEXTURE_2D;
    settings.useDepth=false;settings.useStencil=false;settings.numSamples=0;
    for(auto* f:{&target,&layer,&backup,&display})f->allocate(settings);
    if(!target.isAllocated() || !layer.isAllocated() || !backup.isAllocated())throw std::runtime_error("No se pudo crear el FBO");
}
void BorderRenderer::drawSlice(const BorderMesh& b,const ofJson& p,float time){
    auto c=color(p.at("innerFillColor"));float a=c.a*p.at("innerFillOpacity").get<float>()*p.at("opacity").get<float>();
    if(a>0){ofPath path;path.setFilled(true);for(size_t i=0;i<b.contour.size();++i){auto v=b.contour[i];if(i==0)path.moveTo(v.x,v.y);else path.lineTo(v.x,v.y);}path.close();path.setFillColor(ofFloatColor(c.r*a,c.g*a,c.b*a,a));path.draw();}
    shader.begin();
    shader.setUniform1f("perimeter",b.perimeter);
    for(auto name:{"softness","opacity","coreWidth","gradientRepeat","saturation","brightness"})shader.setUniform1f(name,p.at(name).get<float>());
    shader.setUniform1f("gradientOffset",p.at("gradientOffset").get<float>()+time*p.at("gradientRotation").get<float>()/360);
    shader.setUniform1f("hueShift",time*p.at("hueShiftSpeed").get<float>());
    for(auto name:{"colorA","colorB","coreColor"})shader.setUniform4f(name,color(p.at(name)));
    shader.setUniform1i("colorMode",p.at("colorMode")=="solid"?0:1);
    shader.setUniform1i("profile",p.at("profile")=="hard"?0:p.at("profile")=="soft"?1:2);
    const auto& stops=p.at("gradientStops"); shader.setUniform1i("stopCount",stops.size());
    for(size_t i=0;i<stops.size();++i){shader.setUniform1f("stopPositions["+ofToString(i)+"]",stops[i].at("pos").get<float>());shader.setUniform4f("stopColors["+ofToString(i)+"]",color(stops[i].at("color")));}
    b.mesh.draw();shader.end();
}
void BorderRenderer::render(const Composition& c,const Project& p,float time){
    error.clear();vertices=0;allocate(c.width,c.height);
    ofPushStyle();target.begin();ofClear(0,0,0,0);ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    for(const auto& screen:c.screens){
        if(!p.screenEnabled.value(screen.id,screen.enabled))continue;
        for(const auto& slice:screen.slices){
            auto binding=p.bindings.value(slice.id,ofJson{{"enabled",slice.enabled},{"params",ofJson::object()}});
            if(!binding.value("enabled",slice.enabled))continue;
            try{
                auto params=resolveStyle(p.defaultStyle,p.groups,binding);validateStyle(params);
                const auto& points=params["space"]=="output"?slice.outputPoints:slice.inputPoints;
                ofJson geometry;
                for(auto name:{"space","thickness","offset","alignment","cornerRadius","joinStyle","miterLimit","capStyle","closePath","subdivision","segmentsMode","edgeMask"})geometry[name]=params[name];
                for(auto pt:points)geometry["points"].push_back({pt.x,pt.y});
                std::string key=geometry.dump();auto& entry=cache[slice.id];
                if(entry.key!=key){entry.border=BorderMesh::build(points,params);entry.key=key;++regenerations;}
                vertices+=entry.border.mesh.getNumVertices();
                auto mode=params["blendMode"].get<std::string>();
                if(mode=="screen" || mode=="multiply"){
                    target.end();layer.begin();ofClear(0,0,0,0);
                    glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    drawSlice(entry.border,params,time);layer.end();backup.begin();ofClear(0,0,0,0);ofDisableAlphaBlending();
                    composite.begin();composite.setUniformTexture("sourceTex",layer.getTexture(),0);composite.setUniformTexture("destinationTex",target.getTexture(),1);composite.setUniform1i("blendMode",mode=="multiply"?3:2);
                    // oF flips the projection when writing FBOs, so row zero is input y=0.
                    ofMesh quad;quad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
                    for(auto pair:std::vector<std::pair<glm::vec2,glm::vec2>>{{{0,0},{0,0}},{{float(c.width),0},{1,0}},{{float(c.width),float(c.height)},{1,1}},{{0,float(c.height)},{0,1}}}){quad.addVertex({pair.first.x,pair.first.y,0});quad.addTexCoord(pair.second);}quad.draw();composite.end();backup.end();
                    std::swap(target,backup);target.begin();ofEnableAlphaBlending();
                }else{
                    glBlendFuncSeparate(GL_ONE,mode=="add"?GL_ONE:GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    drawSlice(entry.border,params,time);
                }
            }catch(const std::exception& e){error=slice.name+": "+e.what();}
        }
    }
    target.end();
    display.begin();ofClear(0,0,0,0);ofDisableAlphaBlending();previewShader.begin();
    previewShader.setUniformTexture("frameTex",target.getTexture(),0);
    target.draw(0,0);previewShader.end();display.end();ofPopStyle();
}
}
