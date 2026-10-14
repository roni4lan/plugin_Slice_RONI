#pragma once
#include "BorderStyle.h"
#include <map>
namespace sb {
class StyleRegistry {
public:
    static StyleRegistry& instance();
    void add(std::shared_ptr<BorderStyle>);
    const std::map<std::string,std::shared_ptr<BorderStyle>>& styles() const{return registered;}
    const BorderStyle& get(const std::string&) const;
    void writeInitialPresets() const;
private:
    std::map<std::string,std::shared_ptr<BorderStyle>> registered;
};
}
