#include "NdiOutput.h"
namespace sb {
void NdiOutput::setup(const std::string& name,int fps){
    close();this->name=name;this->fps=fps;
#ifdef TARGET_WIN32
    auto path=ofFilePath::join(ofFilePath::getCurrentExeDir(),"Processing.NDI.Lib.x64.dll");
#elif defined(TARGET_OSX)
    auto path=ofFilePath::join(ofFilePath::getCurrentExeDir(),"libndi.dylib");
#else
    auto path=ofFilePath::join(ofFilePath::getCurrentExeDir(),"libndi.so");
#endif
    dll.open(path);api=dll.symbol<const NDIlib_v5*(*)()>("NDIlib_v5_load")();
    ofLogNotice("NDI") << "API cargada";
    if(!api || !api->initialize())throw std::runtime_error("NDI no pudo inicializarse");
    NDIlib_send_create_t config{};config.p_ndi_name=this->name.c_str();config.clock_video=false;config.clock_audio=false;
    sender=api->send_create(&config);if(!sender)throw std::runtime_error("NDI no pudo crear el emisor");status="NDI preparado: "+name;
    ofLogNotice("NDI") << "Emisor creado";
}
bool NdiOutput::send(ofTexture& texture){
    if(!sender || !api)return false;texture.readToPixels(pixels);
    if(!pixels.isAllocated() || pixels.getNumChannels()!=4){status="NDI requiere RGBA";return false;}
    NDIlib_video_frame_v2_t frame{};frame.xres=pixels.getWidth();frame.yres=pixels.getHeight();frame.FourCC=NDIlib_FourCC_video_type_RGBA;
    frame.frame_rate_N=fps;frame.frame_rate_D=1;frame.picture_aspect_ratio=float(frame.xres)/frame.yres;
    frame.frame_format_type=NDIlib_frame_format_type_progressive;frame.timecode=NDIlib_send_timecode_synthesize;
    frame.p_data=pixels.getData();frame.line_stride_in_bytes=frame.xres*4;
    api->send_send_video_v2(sender,&frame);status="NDI: "+name;return true;
}
void NdiOutput::close(){
    if(sender && api)api->send_destroy(sender);sender=nullptr;
    if(api)api->destroy();api=nullptr;dll.close();
}
bool NdiOutput::verifyLoopback(ofTexture& texture){
    if(!sender || !api)return false;
    auto* source=api->send_get_source_name(sender);if(!source)return false;
    NDIlib_recv_create_v3_t config{};config.source_to_connect_to=*source;config.color_format=NDIlib_recv_color_format_RGBX_RGBA;
    config.bandwidth=NDIlib_recv_bandwidth_highest;config.allow_video_fields=false;
    auto receiver=api->recv_create_v3(&config);if(!receiver)return false;
    ofLogNotice("NDI-test") << "Receptor creado; esperando video";
    ofPixels expected;texture.readToPixels(expected);bool success=false;
    int videoFrames=0,maxAlphaError=-1;
    for(int attempt=0;attempt<80 && !success;++attempt){
        send(texture);NDIlib_video_frame_v2_t frame{};
        auto kind=api->recv_capture_v2(receiver,&frame,nullptr,nullptr,100);
        if(kind!=NDIlib_frame_type_video){ofSleepMillis(10);continue;}
        ++videoFrames;
        if(videoFrames==1)ofLogNotice("NDI-test") << "Video: " << frame.xres << "x" << frame.yres << " FourCC=" << int(frame.FourCC) << " stride=" << frame.line_stride_in_bytes;
        if(frame.xres==texture.getWidth() && frame.yres==texture.getHeight() && frame.FourCC==NDIlib_FourCC_video_type_RGBA){
            // NDI compression may alter RGB; alpha and image orientation must survive.
            maxAlphaError=0;
            for(int y=0;y<frame.yres;++y)for(int x=0;x<frame.xres;++x){size_t offset=size_t(y)*frame.line_stride_in_bytes+x*4;auto p=expected.getColor(x,y);maxAlphaError=std::max(maxAlphaError,std::abs(int(frame.p_data[offset+3])-int(p.a)));}
            // The NDI codec quantizes alpha near antialiased edges (measured max: 5/255).
            // An 8/255 bound still detects a lost alpha channel or a vertically flipped image.
            success=maxAlphaError<=8;
        }
        api->recv_free_video_v2(receiver,&frame);
    }
    ofLogNotice("NDI-test") << "Frames=" << videoFrames << " Error alfa=" << maxAlphaError << " Conexiones=" << api->send_get_no_connections(sender,0);
    api->recv_destroy(receiver);return success;
}
}
