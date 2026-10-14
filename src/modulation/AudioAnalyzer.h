#pragma once
#include "ofMain.h"
#include <array>
#include <mutex>
namespace sb {
struct AudioBands {std::array<float,5> levels{};float rms=0;uint64_t frames=0;};
class AudioAnalyzer : public ofBaseSoundInput {
public:
    ~AudioAnalyzer(){stop();}
    std::vector<ofSoundDevice> devices();
    bool start(int deviceIndex);
    void stop();
    void audioIn(ofSoundBuffer&) override;
    AudioBands bands()const;
    bool active=false;
    std::string status="Audio desactivado";
    static int bandIndex(const std::string& name);
private:
    static constexpr size_t N=1024;
    ofSoundStream stream;
    mutable std::mutex mutex;
    std::array<float,N> ring{};
    size_t cursor=0,count=0;
    AudioBands latest;
    void analyze(unsigned sampleRate);
};
}
