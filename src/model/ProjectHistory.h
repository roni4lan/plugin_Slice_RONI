#pragma once
#include "Composition.h"
#include <deque>
namespace sb {
class ProjectHistory {
public:
    void reset(const Project& p){past.clear();future.clear();last=p;ready=true;}
    void observe(const Project& p,bool editing=false){if(!ready){reset(p);return;}if(editing)return;if(key(p)!=key(last)){past.push_back(last);if(past.size()>64)past.pop_front();future.clear();last=p;}}
    bool undo(Project& p){observe(p);if(past.empty())return false;future.push_back(p);p=past.back();past.pop_back();last=p;return true;}
    bool redo(Project& p){if(future.empty())return false;past.push_back(p);p=future.back();future.pop_back();last=p;return true;}
private:
    Project last;bool ready=false;std::deque<Project> past,future;
    static std::string key(const Project& p){return ofJson{{"xml",p.xmlPath},{"bindings",p.bindings},{"screens",p.screenEnabled},{"groups",p.groups},{"style",p.defaultStyle},{"output",p.output},{"osc",p.osc},{"clock",p.clock},{"audio",p.audio},{"modulation",p.modulation}}.dump();}
};
}
