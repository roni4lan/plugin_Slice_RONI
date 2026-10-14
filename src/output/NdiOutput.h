#pragma once
#include "IOutput.h"
#include "DynamicLibrary.h"
#define NDILIB_CPP_DEFAULT_CONSTRUCTORS 0
#include "Processing.NDI.Lib.h"
namespace sb {
class NdiOutput : public IOutput {
public:
    ~NdiOutput()override{close();}
    void setup(const std::string&,int fps)override;
    bool send(ofTexture&)override;
    void close()override;
    bool verifyLoopback(ofTexture&);
private:
    DynamicLibrary dll;
    const NDIlib_v5* api=nullptr;
    NDIlib_send_instance_t sender=nullptr;
    ofPixels pixels;
    std::string name;
    int fps=60;
};
}
