#include "ofApp.h"
#include <filesystem>
int main(int argc, char** argv) {
    std::string path; bool test = false;
    for (int i=1; i<argc; ++i) { if (std::string(argv[i]) == "--self-test") test = true; else path=argv[i]; }
    if(!path.empty())path=std::filesystem::absolute(path).string();
    auto executableFolder=std::filesystem::absolute(argv[0]).parent_path();
    std::filesystem::current_path(executableFolder);
    if(path.empty() && std::filesystem::exists("data/ejemplo_advanced.xml"))path=std::filesystem::absolute("data/ejemplo_advanced.xml").string();
    ofGLFWWindowSettings settings;
    settings.setGLVersion(3,3); settings.setSize(1280,800);
    settings.title = "Pluguin Slice RONI RESO";
    auto window=ofCreateWindow(settings);
    ofRunApp(window,std::make_shared<ofApp>(path,test));
    return ofRunMainLoop();
}
