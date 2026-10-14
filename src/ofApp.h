#pragma once
#include "ofMain.h"
#include "ofxImGui.h"
#include "model/Composition.h"
#include "render/BorderRenderer.h"
#include "output/OutputManager.h"
class ofApp : public ofBaseApp {
public:
    explicit ofApp(std::string initial = {}, bool selfTest = false): initialPath(std::move(initial)), testMode(selfTest) {}
    void setup() override;
    void draw() override;
    void dragEvent(ofDragInfo info) override;
    void keyPressed(int key) override;
    void exit() override { output.close(); }
private:
    ofxImGui::Gui gui;
    sb::Composition composition;
    sb::Project project;
    sb::BorderRenderer renderer;
    sb::OutputManager output;
    bool showBorders=true;
    std::string initialPath, projectPath, selected, status = "Abrir un XML de Advanced Output";
    std::string lastRenderError;
    bool testMode = false, loaded = false;
    bool fitPending = true, outputSpace = false;
    float zoom = 1;
    glm::vec2 pan{0,0};
    void open(const std::string&);
    void dialogOpen();
    void saveProject();
    void sidebar();
    void preview();
    void selfTest();
};
