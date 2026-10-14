#pragma once
#include "IOutput.h"
#include "DynamicLibrary.h"
#ifdef TARGET_WIN32
#include "SpoutLibrary.h"
#endif
namespace sb {
class SpoutOutput : public IOutput {
public:
    ~SpoutOutput()override{close();}
    void setup(const std::string&,int fps)override;
    bool send(ofTexture&)override;
    void close()override;
    bool verifyLoopback(ofTexture& source);
private:DynamicLibrary dll;
#ifdef TARGET_WIN32
    SPOUTLIBRARY* sender=nullptr;
    using Factory=SPOUTLIBRARY*(WINAPI*)(void);
    Factory factory=nullptr;
#endif
};
}
