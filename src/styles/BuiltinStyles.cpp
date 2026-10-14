#include "StyleRegistry.h"
namespace sb { namespace {
// New styles can register from their own translation unit without editing the renderer.
class PresetStyle : public BorderStyle {
public:
    PresetStyle(std::string name,ofJson values):id(std::move(name)),values(std::move(values)){this->values["presetName"]=id;}
    std::string name() const override{return id;}
    ofJson defaults() const override{return values;}
private:std::string id;ofJson values;
};
const bool registered=[](){
    auto& registry=StyleRegistry::instance();
    registry.add(std::make_shared<PresetStyle>("Solid",ofJson{{"pattern","solid"},{"animation","none"},{"profile","hard"}}));
    registry.add(std::make_shared<PresetStyle>("Dashed",ofJson{{"pattern","dashed"},{"dashSpeed",80.0},{"dashRoundCaps",true}}));
    registry.add(std::make_shared<PresetStyle>("Chase",ofJson{{"animation","chase"},{"chaseLength",0.08},{"chaseTail",0.28},{"glowEnabled",true},{"glowRadius",18.0}}));
    registry.add(std::make_shared<PresetStyle>("Neon",ofJson{{"profile","gaussian"},{"softness",0.5},{"coreWidth",0.25},{"glowEnabled",true},{"glowRadius",36.0},{"glowIntensity",1.6},{"flicker",0.15}}));
    registry.add(std::make_shared<PresetStyle>("Gradient",ofJson{{"colorMode","gradientPerimeter"},{"gradientRotation",70.0},{"hueShiftSpeed",0.05}}));
    registry.add(std::make_shared<PresetStyle>("DoubleLine",ofJson{{"profile","doubleLine"},{"lineCount",2},{"lineGap",10.0},{"colorMode","gradientPerimeter"},{"gradientRotation",28.0}}));
    registry.add(std::make_shared<PresetStyle>("Glitch",ofJson{{"noiseEnabled",true},{"noiseAffects",5},{"noiseAmount",12.0},{"glitchAmount",0.7},{"jitter",2.0},{"colorA",{1.0,0.1,0.4,1.0}}}));
    registry.add(std::make_shared<PresetStyle>("Pulse",ofJson{{"animation","pulse"},{"pulseDepth",0.85},{"pulseRate",1.5},{"glowEnabled",true},{"glowRadius",20.0}}));
    return true;
}();
}}
