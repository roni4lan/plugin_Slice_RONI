#pragma once
#include "../model/Composition.h"
namespace sb {
class ResolumeXmlLoader {
public:
    static Composition load(const std::string& path);
};
}
