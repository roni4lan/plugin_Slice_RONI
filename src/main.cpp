#include "ofApp.h"
int main(int argc, char** argv) {
    std::string path; bool test = false;
    for (int i=1; i<argc; ++i) { if (std::string(argv[i]) == "--self-test") test = true; else path=argv[i]; }
    ofGLFWWindowSettings settings;
    settings.setGLVersion(3,3); settings.setSize(1280,800);
    settings.title = "Pluguin Slice RONI RESO";
    auto window=ofCreateWindow(settings);
    ofRunApp(window,std::make_shared<ofApp>(path,test));
    return ofRunMainLoop();
}
