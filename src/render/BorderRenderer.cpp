#include "BorderRenderer.h"
#include "../model/StyleParams.h"
#include "../styles/StyleRegistry.h"
namespace sb { namespace {
glm::vec4 color(const ofJson& value){return {value[0].get<float>(),value[1].get<float>(),value[2].get<float>(),value[3].get<float>()};}
}
void BorderRenderer::setup(){
    ofDisableArbTex();
    glow.setup();
    shaders.get("shaders/border_solid.frag");
    if(!composite.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/composite.frag")) ||
       !previewShader.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/preview.frag")))
        throw std::runtime_error("No se pudieron cargar los shaders");
}
void BorderRenderer::allocate(int w,int h){
    if(target.isAllocated() && target.getWidth()==w && target.getHeight()==h)return;
    ofFbo::Settings settings;settings.width=w;settings.height=h;settings.internalformat=GL_RGBA8;settings.textureTarget=GL_TEXTURE_2D;
    settings.useDepth=false;settings.useStencil=false;settings.numSamples=0;
    for(auto* f:{&target,&layer,&backup,&display})f->allocate(settings);
    settings.internalformat=GL_RGBA16F;bloomSource.allocate(settings);
    if(!target.isAllocated() || !layer.isAllocated() || !backup.isAllocated())throw std::runtime_error("No se pudo crear el FBO");
}
void BorderRenderer::drawSlice(const BorderMesh& b,const ofJson& p,float time,float phase,float gain){
    auto c=color(p.at("innerFillColor"));float a=c.a*p.at("innerFillOpacity").get<float>()*p.at("opacity").get<float>();
    if(a>0){ofPath path;path.setFilled(true);for(size_t i=0;i<b.contour.size();++i){auto v=b.contour[i];if(i==0)path.moveTo(v.x,v.y);else path.lineTo(v.x,v.y);}path.close();path.setFillColor(ofFloatColor(c.r*a,c.g*a,c.b*a,a));path.draw();}
    const auto& style=StyleRegistry::instance().get(p.value("presetName",std::string("Solid")));
    auto& shader=shaders.get(style.fragmentShader());shader.begin();
    for(const auto& meta:parameterRegistry()){
        auto v=p.at(meta.name);
        if(meta.type=="float")shader.setUniform1f(meta.name,v.get<float>());
        else if(meta.type=="int")shader.setUniform1i(meta.name,v.get<int>());
        else if(meta.type=="bool")shader.setUniform1i(meta.name,v.get<bool>());
        else if(meta.type=="color")shader.setUniform4f(meta.name,color(v));
        else if(meta.type=="enum")shader.setUniform1i(meta.name,std::find(meta.options.begin(),meta.options.end(),v.get<std::string>())-meta.options.begin());
    }
    shader.setUniform1f("time",time);shader.setUniform1f("phaseOffset",phase);shader.setUniform1f("bloomGain",gain);
    shader.setUniform1f("perimeter",b.perimeter);
    for(auto name:{"softness","opacity","coreWidth","gradientRepeat","saturation","brightness"})shader.setUniform1f(name,p.at(name).get<float>());
    shader.setUniform1f("gradientOffset",p.at("gradientOffset").get<float>()+time*p.at("gradientRotation").get<float>()/360);
    shader.setUniform1f("hueShift",time*p.at("hueShiftSpeed").get<float>());
    for(auto name:{"colorA","colorB","coreColor"})shader.setUniform4f(name,color(p.at(name)));
    shader.setUniform1i("colorMode",p.at("colorMode")=="solid"?0:1);
    const auto& stops=p.at("gradientStops"); shader.setUniform1i("stopCount",stops.size());
    for(size_t i=0;i<stops.size();++i){shader.setUniform1f("stopPositions["+ofToString(i)+"]",stops[i].at("pos").get<float>());shader.setUniform4f("stopColors["+ofToString(i)+"]",color(stops[i].at("color")));}
    style.uniforms(shader,p,time);b.mesh.draw();shader.end();
}
void BorderRenderer::render(const Composition& c,const Project& p,float time){
    error.clear();vertices=0;allocate(c.width,c.height);
    std::vector<const Slice*> all;for(const auto& screen:c.screens)for(const auto& slice:screen.slices)all.push_back(&slice);
    auto center=[](const Slice& slice){glm::vec2 sum{0,0};for(auto pt:slice.inputPoints)sum+=pt;return sum/float(slice.inputPoints.size());};
    auto score=[&](const Slice& slice,const std::string& order,int seed){auto pt=center(slice);if(order=="byX")return double(pt.x);if(order=="byY")return double(pt.y);if(order=="byDistanceFromCenter")return double(glm::distance(pt,glm::vec2(c.width/2.f,c.height/2.f)));if(order=="random"){uint32_t hash=2166136261u^seed;for(unsigned char ch:slice.id){hash^=ch;hash*=16777619u;}return double(hash);}return double(std::find(all.begin(),all.end(),&slice)-all.begin());};
    float bloomRadius=0;int bloomPasses=1;
    ofPushStyle();bloomSource.begin();ofClear(0,0,0,0);bloomSource.end();target.begin();ofClear(0,0,0,0);ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    for(const auto& screen:c.screens){
        if(!p.screenEnabled.value(screen.id,screen.enabled))continue;
        for(const auto& slice:screen.slices){
            auto binding=p.bindings.value(slice.id,ofJson{{"enabled",slice.enabled},{"params",ofJson::object()}});
            if(!binding.value("enabled",slice.enabled))continue;
            try{
                auto params=resolveStyle(p.defaultStyle,p.groups,binding);validateStyle(params);
                auto order=params["phaseSpreadOrder"].get<std::string>();int seed=params["seed"];double ownScore=score(slice,order,seed);int rank=0;
                for(auto other:all){double otherScore=score(*other,order,seed);if(otherScore<ownScore || (otherScore==ownScore && other->id<slice.id))++rank;}
                float phase=params["phaseSpread"].get<float>()*rank/std::max(1,int(all.size())-1);
                const auto& points=params["space"]=="output"?slice.outputPoints:slice.inputPoints;
                ofJson geometry;
                for(auto name:{"space","thickness","offset","alignment","cornerRadius","joinStyle","miterLimit","capStyle","closePath","subdivision","segmentsMode","edgeMask","profile","lineCount","lineGap","glowEnabled","glowMode","glowRadius","pattern","tickLength"})geometry[name]=params[name];
                geometry["distortionResample"]=params["noiseEnabled"].get<bool>() || params["jitter"].get<float>()>0 || params["glitchAmount"].get<float>()>0 || params["pattern"]=="wave" || params["pattern"]=="zigzag";
                for(auto pt:points)geometry["points"].push_back({pt.x,pt.y});
                std::string key=geometry.dump();auto& entry=cache[slice.id];
                if(entry.key!=key){entry.border=BorderMesh::build(points,params);entry.key=key;++regenerations;}
                vertices+=entry.border.mesh.getNumVertices();
                if(params["glowEnabled"]==true && params["glowMode"]=="bloom" && params["glowRadius"].get<float>()>0){
                    bloomRadius=std::max(bloomRadius,params["glowRadius"].get<float>());bloomPasses=std::max(bloomPasses,params["glowPasses"].get<int>());
                    auto haloParams=params;if(params["glowUseBorderColor"]==false){haloParams["colorA"]=params["glowColor"];haloParams["colorB"]=params["glowColor"];haloParams["colorMode"]="solid";haloParams["gradientStops"]=ofJson::array();}
                    target.end();bloomSource.begin();glBlendFuncSeparate(GL_ONE,GL_ONE,GL_ONE,GL_ONE);
                    drawSlice(entry.border,haloParams,time,phase,params["glowIntensity"].get<float>());bloomSource.end();target.begin();
                }
                auto mode=params["blendMode"].get<std::string>();
                if(mode=="screen" || mode=="multiply"){
                    target.end();layer.begin();ofClear(0,0,0,0);
                    glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    drawSlice(entry.border,params,time,phase);layer.end();backup.begin();ofClear(0,0,0,0);ofDisableAlphaBlending();
                    composite.begin();composite.setUniformTexture("sourceTex",layer.getTexture(),0);composite.setUniformTexture("destinationTex",target.getTexture(),1);composite.setUniform1i("blendMode",mode=="multiply"?3:2);
                    // oF flips the projection when writing FBOs, so row zero is input y=0.
                    ofMesh quad;quad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
                    for(auto pair:std::vector<std::pair<glm::vec2,glm::vec2>>{{{0,0},{0,0}},{{float(c.width),0},{1,0}},{{float(c.width),float(c.height)},{1,1}},{{0,float(c.height)},{0,1}}}){quad.addVertex({pair.first.x,pair.first.y,0});quad.addTexCoord(pair.second);}quad.draw();composite.end();backup.end();
                    std::swap(target,backup);target.begin();ofEnableAlphaBlending();
                }else{
                    glBlendFuncSeparate(GL_ONE,mode=="add"?GL_ONE:GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    drawSlice(entry.border,params,time,phase);
                }
            }catch(const std::exception& e){error=slice.name+": "+e.what();}
        }
    }
    target.end();
    if(bloomRadius>0){glow.apply(bloomSource,target,backup,bloomRadius,bloomPasses);std::swap(target,backup);}
    display.begin();ofClear(0,0,0,0);ofDisableAlphaBlending();previewShader.begin();
    previewShader.setUniformTexture("frameTex",target.getTexture(),0);
    target.draw(0,0);previewShader.end();display.end();ofPopStyle();
}
}
