#include "SpoutOutput.h"
namespace sb {
void SpoutOutput::setup(const std::string& name,int){
    close();
#ifdef TARGET_WIN32
    // The DLL is built from the same source tree as this interface header.
    dll.open(ofFilePath::join(ofFilePath::getCurrentExeDir(),"SpoutLibrary.dll"));
    factory=dll.symbol<Factory>("GetSpout");sender=factory();
    ofLogNotice("Spout") << "Factory creada";
    if(!sender)throw std::runtime_error("Spout no pudo crear el emisor");
    sender->SetSenderName(name.c_str());sender->SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    ofLogNotice("Spout") << "Nombre y formato configurados";
    status="Spout preparado";
#else
    throw std::runtime_error("Spout solo esta disponible en Windows");
#endif
}
bool SpoutOutput::send(ofTexture& texture){
#ifdef TARGET_WIN32
    if(!sender)return false;const auto& data=texture.getTextureData();
    // The oF FBO stores the top of the image in row zero; do not invert it again.
    bool sent=sender->SendTexture(data.textureID,data.textureTarget,texture.getWidth(),texture.getHeight(),false,0);
    status=sent?"Spout: "+std::string(sender->GetName()):"Spout: envio fallido; comprobar GPU compatible";return sent;
#else
    return false;
#endif
}
void SpoutOutput::close(){
#ifdef TARGET_WIN32
    if(sender){sender->ReleaseSender();sender->Release();sender=nullptr;}factory=nullptr;
#endif
    dll.close();
}
bool SpoutOutput::verifyLoopback(ofTexture& source){
#ifdef TARGET_WIN32
    if(!sender || !factory || !send(source))return false;
    ofLogNotice("Spout-test") << "Primer envio correcto";
    auto* receiver=factory();if(!receiver)return false;
    receiver->SetReceiverName(sender->GetName());
    ofLogNotice("Spout-test") << "Receptor creado";
    ofPixels expected,actual;source.readToPixels(expected);actual.allocate(source.getWidth(),source.getHeight(),OF_PIXELS_RGBA);
    bool success=false;
    for(int i=0;i<20 && !success;++i){send(source);receiver->ReceiveTexture();
        if(receiver->GetSenderWidth()!=source.getWidth() || receiver->GetSenderHeight()!=source.getHeight())continue;
        if(receiver->ReceiveImage(actual.getData(),GL_RGBA,false,0)){
            success=actual.size()==expected.size();
            if(success)for(size_t k=0;k<actual.size();++k)if(std::abs(int(actual[k])-int(expected[k]))>2){success=false;break;}
        }ofSleepMillis(5);
    }
    ofLogNotice("Spout-test") << "Round trip: " << success;
    receiver->ReleaseReceiver();receiver->Release();return success;
#else
    return false;
#endif
}
}
