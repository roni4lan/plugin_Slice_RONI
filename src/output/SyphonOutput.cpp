#include "SyphonOutput.h"
namespace sb {
void SyphonOutput::setup(const std::string& name,int){
#ifdef SB_HAS_SYPHON
    server=std::make_unique<ofxSyphonServer>();server->setName(name);status="Syphon: "+name;
#else
    throw std::runtime_error("Syphon requiere macOS y ofxSyphon agregado al proyecto");
#endif
}
bool SyphonOutput::send(ofTexture& texture){
#ifdef SB_HAS_SYPHON
    if(!server)return false;server->publishTexture(&texture);return true;
#else
    return false;
#endif
}
void SyphonOutput::close(){
#ifdef SB_HAS_SYPHON
    server.reset();
#endif
}
}
