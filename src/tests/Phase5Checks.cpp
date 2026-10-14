#include "Phase5Checks.h"
#include "../model/StyleParams.h"
#include "../modulation/LFO.h"
#include "../modulation/Envelope.h"
#include "../modulation/BeatClock.h"
#include "../modulation/AudioAnalyzer.h"
#include "../modulation/ModulationMatrix.h"
#include "../control/OscServer.h"
#include "../io/ProjectIO.h"
#include "ofxOscSender.h"
namespace sb { namespace {
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
bool closeEnough(float a,float b){return std::abs(a-b)<0.001f;}
}
ofJson phase5Checks(const Composition& fixture,const std::string& xmlPath,BorderRenderer& renderer){
    ofJson report;
    require(closeEnough(LFO::value(0.25,1,"sine"),1),"LFO seno incorrecto");
    Envelope envelope;envelope.trigger(0);require(closeEnough(envelope.value(0.01,0.02,0.3,0.5,0.3),0.5),"Attack incorrecto");
    require(closeEnough(envelope.value(0.17,0.02,0.3,0.5,0.3),0.5),"Decay incorrecto");require(envelope.value(0.5,0.02,0.3,0.5,0.3)==0,"AD no termina");
    envelope.gate(true,0,0.02,0.3,0.4);require(closeEnough(envelope.value(0.5,0.02,0.3,0.4,0.3,true),0.4),"Sustain incorrecto");
    envelope.gate(false,0.5,0.02,0.3,0.4);require(closeEnough(envelope.value(0.65,0.02,0.3,0.4,0.3,true),0.2),"Release incorrecto");
    BeatClock clock;clock.setBpm(120,0);require(closeEnough(clock.beats(0.25),0.5),"Reloj BPM incorrecto");
    clock.tap(0);clock.tap(0.5);clock.tap(1);require(closeEnough(clock.bpm,120),"Tap tempo incorrecto");
    Project base;base.defaultStyle=styleDefaults();base.defaultStyle["thickness"]=195.0;
    auto route=ofJson{{"id","lfo"},{"scope","global"},{"target","thickness"},{"source","LFO"},{"frequency",1.0},{"amount",10.0}};
    auto inverse=route;inverse["id"]="inverse";inverse["amount"]=-10.0;base.modulation=ofJson::array({route,inverse});
    ModulationMatrix matrix;AudioBands silent;
    require(closeEnough(matrix.apply(base,fixture,0.25,silent,clock,{}).defaultStyle["thickness"],195),"Las rutas se clamplean antes de sumarse");
    base.defaultStyle["thickness"]=6.0;base.modulation=ofJson::array({route});
    require(closeEnough(matrix.apply(base,fixture,0.25,silent,clock,{}).defaultStyle["thickness"],16),"LFO no modula el parametro");
    base.modulation[0]["source"]="Beat";
    BeatClock beatClock;beatClock.setBpm(120,0);
    require(closeEnough(matrix.apply(base,fixture,0.125,silent,beatClock,{}).defaultStyle["thickness"],16),"Fuente Beat no sigue BPM");
    base.modulation[0]["source"]="LFO";base.modulation[0]["target"]="colorA.r";base.modulation[0]["amount"]=0.3;
    require(closeEnough(matrix.apply(base,fixture,0.25,silent,beatClock,{}).defaultStyle["colorA"][0],0.3),"Color numerico no se modula");
    AudioAnalyzer analyzer;ofSoundBuffer signal;signal.setNumChannels(1);signal.setSampleRate(48000);signal.resize(1024);
    for(size_t i=0;i<signal.size();++i)signal[i]=0.5f*std::sin(TWO_PI*2*i/1024);
    analyzer.audioIn(signal);auto bands=analyzer.bands();
    require(bands.levels[0]>0.7f && bands.levels[0]>bands.levels[2]+0.2f && closeEnough(bands.rms,std::sqrt(0.125f)),"FFT no detecta el tono grave");
    base.defaultStyle["opacity"]=0.1;base.modulation[0]={{"id","audio"},{"scope","global"},{"target","opacity"},{"source","Audio"},{"audioBand","bass"},{"smoothing",0.0},{"amount",0.5}};
    require(closeEnough(matrix.apply(base,fixture,0,silent,beatClock,{}).defaultStyle["opacity"],0.1f),"Audio silencioso altera parametro");
    require(matrix.apply(base,fixture,0,bands,beatClock,{}).defaultStyle["opacity"].get<float>()>0.4,"Audio no modula el parametro");
    // Exercise actual UDP delivery, not only the message dispatcher.
    Project controlled;controlled.xmlPath=xmlPath;controlled.groups["TestGroup"]=ofJson::object();int port=23000+(ofGetSystemTimeMillis()%1000);
    controlled.osc={{"enabled",true},{"port",port}};OscServer server;ModulationMatrix controlledMatrix;BeatClock controlledClock;
    server.update(controlled,fixture,controlledClock,controlledMatrix,0);ofxOscSender sender;require(sender.setup("127.0.0.1",port),"Cliente OSC no inicializa");
    auto sendFloat=[&](const std::string& address,float value){ofxOscMessage msg;msg.setAddress(address);msg.addFloatArg(value);require(sender.sendMessage(msg,false),"No se envio OSC");};
    auto sliceId=fixture.screens.at(0).slices.at(0).id;
    ofxOscMessage style;style.setAddress("/style/"+sliceId);style.addStringArg("Neon");sender.sendMessage(style,false);
    sendFloat("/global/thickness",23);sendFloat("/slice/"+sliceId+"/speed",2.5);sendFloat("/group/TestGroup/opacity",0.25);sendFloat("/clock/bpm",150);sendFloat("/mod/1",0.4);sendFloat("/output/enable",1);
    ofxOscMessage color;color.setAddress("/global/colorA");for(float c:{0.2f,0.6f,0.9f,0.4f})color.addFloatArg(c);sender.sendMessage(color,false);
    for(int i=0;i<30 && server.messages<8;++i){ofSleepMillis(10);server.update(controlled,fixture,controlledClock,controlledMatrix,0.5);}
    require(server.messages==8 && closeEnough(controlled.defaultStyle["thickness"],23) && closeEnough(controlled.bindings[sliceId]["params"]["speed"],2.5),"OSC no cambia grosor/velocidad");
    require(controlled.bindings[sliceId]["styleName"]=="Neon" && closeEnough(controlled.groups["TestGroup"]["opacity"],0.25) && closeEnough(controlled.defaultStyle["colorA"][0],0.2),"OSC no cambia estilo/color/grupo");
    require(closeEnough(controlledClock.bpm,150) && closeEnough(server.values["/mod/1"],0.4) && controlled.output["enabled"]==true,"OSC reloj/salida incorrectos");
    ofxOscMessage invalid;invalid.setAddress("/global/thickness");invalid.addStringArg("malformed");require(!server.dispatch(invalid,controlled,fixture,controlledClock,controlledMatrix,1),"OSC invalido no rechazado");
    server.stop();controlled.modulation=base.modulation;controlled.audio={{"enabled",false},{"deviceIndex",0}};
    auto saved=ofToDataPath("projects/phase5-self-test.json",true);ProjectIO::save(controlled,saved);auto reopened=ProjectIO::load(saved);
    require(reopened.modulation==controlled.modulation && reopened.clock==controlled.clock && reopened.audio==controlled.audio,"Proyecto no restaura modulacion/reloj/audio");
    Composition sample;sample.width=640;sample.height=480;Screen screen;screen.id="bpm-test";Slice slice;slice.id="bpm-slice";slice.name="BPM";slice.inputPoints={{100,100},{500,100},{500,400},{100,400}};slice.outputPoints=slice.inputPoints;screen.slices={slice};sample.screens={screen};
    Project pulse;pulse.defaultStyle=styleDefaults();pulse.defaultStyle.update({{"animation","pulse"},{"pulseDepth",1.0},{"syncMode","bpm"},{"bpm",120.0},{"thickness",16.0}});
    ModulationMatrix beatMatrix;BeatClock renderClock;renderClock.setBpm(120,0);ofPixels pixels;
    renderer.render(sample,beatMatrix.apply(pulse,sample,0.125,silent,renderClock,{}),0.125);renderer.frame().readToPixels(pixels);int high=pixels.getColor(100,200).a;
    renderer.render(sample,beatMatrix.apply(pulse,sample,0.375,silent,renderClock,{}),0.375);renderer.frame().readToPixels(pixels);int low=pixels.getColor(100,200).a;
    require(high>240 && low<10,"Renderer no sigue la fase BPM");
    report={{"lfo",true},{"adAdsr",true},{"tapTempo",true},{"modulationSumBeforeClamp",true},{"audioFftTone",true},{"audioParameterReaction",true},{"oscUdpEightMessages",true},{"bpmRenderedPulse",true},{"controlProjectRoundTrip",true}};
    AudioAnalyzer live;auto devices=live.devices();report["audioInputDevices"]=devices.size();
    if(!devices.empty() && live.start(0)){
        for(int i=0;i<40 && live.bands().frames<1024;++i)ofSleepMillis(20);
        auto captured=live.bands();live.stop();require(captured.frames>=1024,"Entrada de audio sin callbacks");report["liveAudioCapturedFrames"]=captured.frames;
    }else report["liveAudioStatus"]=live.status;
    return report;
}
}

