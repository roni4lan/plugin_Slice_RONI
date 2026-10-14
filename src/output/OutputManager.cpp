#include "OutputManager.h"
#include "SpoutOutput.h"
#include "NdiOutput.h"
#include "SyphonOutput.h"
namespace sb {
void OutputManager::update(const ofJson& settings,ofFbo& premultiplied,ofFbo& straight){
    if(!settings.value("enabled",false)){if(backend || !signature.empty())close();return;}
    try{
        int fps=settings.value("fps",60);auto type=settings.value("type",std::string("Spout"));
        if(fps<1 || fps>120)throw std::runtime_error("FPS de salida fuera de rango (1..120)");
        auto name=settings.value("name",std::string("RONI RESO"));
        if(name.empty() || name.size()>200)throw std::runtime_error("Nombre de emisor invalido");
        if(settings.value("resolutionMode",std::string("composition"))=="custom"){
            int width=settings.value("width",int(premultiplied.getWidth())),height=settings.value("height",int(premultiplied.getHeight()));
            GLint maxTexture=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexture);
            if(width<1 || height<1 || width>maxTexture || height>maxTexture)throw std::runtime_error("Resolucion de salida fuera de los limites de la GPU");
        }
        auto key=settings.dump();
        if(key!=signature){
            close();signature=key;
            if(type=="Spout")backend=std::make_unique<SpoutOutput>();
            else if(type=="NDI")backend=std::make_unique<NdiOutput>();
            else if(type=="Syphon")backend=std::make_unique<SyphonOutput>();
            else throw std::runtime_error("Salida desconocida");
            backend->setup(settings.value("name",std::string("RONI RESO")),fps);
            lastSent=-1;windowStart=ofGetElapsedTimef();windowFrames=0;
        }
        if(!backend)return;
        double now=ofGetElapsedTimef();if(lastSent>=0 && now-lastSent<1.0/fps-0.0005)return;
        // NDI explicitly requires straight RGBA. Spout/Syphon can retain the renderer's alpha.
        bool directAlpha=type=="NDI" || settings.value("alphaMode",std::string("premultiplied"))=="straight";
        auto& source=directAlpha?straight:premultiplied;
        ofTexture* texture=&source.getTexture();
        if(settings.value("resolutionMode",std::string("composition"))=="custom"){
            int w=settings.value("width",int(source.getWidth())),h=settings.value("height",int(source.getHeight()));
            GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);if(w<1 || h<1 || w>limit || h>limit)throw std::runtime_error("Resolucion de salida supera el limite de la GPU");
            if(!scaled.isAllocated() || scaled.getWidth()!=w || scaled.getHeight()!=h){ofFbo::Settings s;s.width=w;s.height=h;s.internalformat=GL_RGBA8;s.textureTarget=GL_TEXTURE_2D;s.useDepth=false;s.useStencil=false;scaled.allocate(s);}
            ofPushStyle();scaled.begin();ofClear(0,0,0,0);ofDisableAlphaBlending();ofSetColor(255);source.draw(0,0,w,h);scaled.end();ofPopStyle();texture=&scaled.getTexture();
        }
        if(backend->send(*texture)){
            ++sentFrames;++windowFrames;
            // Advance the scheduled timestamp instead of accumulating frame jitter.
            // After a long stall restart at now, avoiding a burst of catch-up frames.
            lastSent=lastSent<0 || now-lastSent>2.0/fps?now:lastSent+1.0/fps;
        }status=backend->status;
        if(now-windowStart>=1){measuredFps=windowFrames/(now-windowStart);windowFrames=0;windowStart=now;}
    }catch(const std::exception& e){status=e.what();if(backend){backend->close();backend.reset();}}
}
}
