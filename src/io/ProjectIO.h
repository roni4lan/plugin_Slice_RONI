#pragma once
#include "../model/Composition.h"
namespace sb {
class ProjectIO {
public:
    static void save(const Project&, const std::string& path);
    static Project load(const std::string& path);
};
}
