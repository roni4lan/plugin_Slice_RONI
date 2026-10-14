#pragma once
#include "IOutput.h"
namespace sb {
class OutputManager {
public:
    void update(const ofJson& settings,ofFbo& premultiplied,ofFbo& straight);
    void close(){if(backend)backend->close();backend.reset();signature.clear();status="Salida desactivada";}
    std::string status="Salida desactivada";
    uint64_t sentFrames=0;
    double measuredFps=0;
private:
    std::unique_ptr<IOutput> backend;
    ofFbo scaled;
    std::string signature;
    double lastSent=-1,windowStart=0;
    int windowFrames=0;
};
}
