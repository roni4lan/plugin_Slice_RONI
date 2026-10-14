#include "GlowPass.h"
#include "ShaderManager.h"
namespace sb {
void GlowPass::setup(){
    if(!blur.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/blur.frag")) || !mix.load(ofToDataPath("shaders/border.vert"),ofToDataPath("shaders/glow_mix.frag")) || !linkedShader(blur) || !linkedShader(mix))throw std::runtime_error("Shader de halo invalido");
}
void GlowPass::apply(ofFbo& source,ofFbo& borders,ofFbo& destination,float radius,int passes){
    int w=std::max(1,int(source.getWidth())/2),h=std::max(1,int(source.getHeight())/2);
    if(!ping.isAllocated() || ping.getWidth()!=w || ping.getHeight()!=h){ofFbo::Settings settings;settings.width=w;settings.height=h;settings.internalformat=GL_RGBA16F;settings.textureTarget=GL_TEXTURE_2D;settings.useDepth=false;settings.useStencil=false;for(auto* f:{&ping,&pong})f->allocate(settings);}
    ofPushStyle();ofDisableAlphaBlending();ofSetColor(255);
    ping.begin();ofClear(0,0,0,0);source.draw(0,0,w,h);ping.end();
    float tap=radius*0.5f/(8*std::sqrt(float(passes)));
    for(int i=0;i<passes;++i){
        pong.begin();ofClear(0,0,0,0);blur.begin();blur.setUniformTexture("frameTex",ping.getTexture(),0);blur.setUniform2f("stepUV",tap/w,0);ping.draw(0,0);blur.end();pong.end();
        ping.begin();ofClear(0,0,0,0);blur.begin();blur.setUniformTexture("frameTex",pong.getTexture(),0);blur.setUniform2f("stepUV",0,tap/h);pong.draw(0,0);blur.end();ping.end();
    }
    destination.begin();ofClear(0,0,0,0);mix.begin();mix.setUniformTexture("borderTex",borders.getTexture(),0);mix.setUniformTexture("glowTex",ping.getTexture(),1);borders.draw(0,0);mix.end();destination.end();ofPopStyle();
}
}
