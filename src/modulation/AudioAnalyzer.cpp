#include "AudioAnalyzer.h"
#include <complex>
namespace sb {
std::vector<ofSoundDevice> AudioAnalyzer::devices(){auto all=stream.getDeviceList();std::vector<ofSoundDevice> inputs;for(auto d:all)if(d.inputChannels>0)inputs.push_back(d);return inputs;}
bool AudioAnalyzer::start(int index){
    stop();try{auto available=devices();if(index<0 || index>=int(available.size()))throw std::runtime_error("Entrada de audio no disponible");
        auto device=available[index];ofSoundStreamSettings settings;settings.setInDevice(device);settings.setInListener(this);settings.numInputChannels=1;settings.numOutputChannels=0;settings.bufferSize=512;settings.numBuffers=3;
        settings.sampleRate=48000;if(!device.sampleRates.empty() && std::find(device.sampleRates.begin(),device.sampleRates.end(),48000)==device.sampleRates.end())settings.sampleRate=device.sampleRates.front();
        active=stream.setup(settings);status=active?"Audio: "+device.name:"No se pudo abrir audio";return active;
    }catch(const std::exception& e){status=e.what();return false;}
}
void AudioAnalyzer::stop(){if(active){stream.stop();stream.close();}active=false;status="Audio desactivado";}
int AudioAnalyzer::bandIndex(const std::string& name){if(name=="lowMid")return 1;if(name=="mid")return 2;if(name=="high")return 3;if(name=="all")return 4;return 0;}
AudioBands AudioAnalyzer::bands()const{std::lock_guard<std::mutex> lock(mutex);return latest;}
void AudioAnalyzer::audioIn(ofSoundBuffer& buffer){
    if(buffer.getNumChannels()==0 || buffer.getSampleRate()==0)return;
    std::lock_guard<std::mutex> lock(mutex);
    for(size_t i=0;i<buffer.getNumFrames();++i){float value=0;for(size_t c=0;c<buffer.getNumChannels();++c)value+=buffer[i*buffer.getNumChannels()+c];value/=buffer.getNumChannels();ring[cursor]=value;cursor=(cursor+1)%N;count=std::min(N,count+1);}
    latest.frames+=buffer.getNumFrames();if(count==N)analyze(buffer.getSampleRate());
}
void AudioAnalyzer::analyze(unsigned rate){
    std::array<std::complex<float>,N> fft;float rms=0;
    for(size_t i=0;i<N;++i){float sample=ring[(cursor+i)%N];rms+=sample*sample;fft[i]=sample*(0.5f-0.5f*std::cos(TWO_PI*i/(N-1)));}
    for(size_t i=1,j=0;i<N;++i){size_t bit=N>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(fft[i],fft[j]);}
    for(size_t length=2;length<=N;length<<=1){auto step=std::polar(1.f,-float(TWO_PI/length));for(size_t i=0;i<N;i+=length){std::complex<float> w{1,0};for(size_t j=0;j<length/2;++j){auto a=fft[i+j],b=fft[i+j+length/2]*w;fft[i+j]=a+b;fft[i+j+length/2]=a-b;w*=step;}}}
    std::array<float,5> energy{};
    for(size_t k=1;k<N/2;++k){float hz=float(k)*rate/N,amplitude=std::abs(fft[k])*4/N;
        if(hz<20 || hz>20000)continue;int band=hz<250?0:hz<500?1:hz<2000?2:3;energy[band]+=amplitude*amplitude;energy[4]+=amplitude*amplitude;
    }
    for(int band=0;band<5;++band){float amplitude=std::sqrt(energy[band]/3);latest.levels[band]=ofClamp((20*std::log10(std::max(amplitude,0.000001f))+60)/60,0.f,1.f);}
    latest.rms=std::sqrt(rms/N);
}
}
