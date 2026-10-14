#include "ResolumeXmlLoader.h"
#include "pugixml.hpp"
#include <cmath>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>

namespace sb { namespace {
std::string required(pugi::xml_node n, const char* attr) {
    auto a = n.attribute(attr);
    if (!a || std::string(a.value()).empty())
        throw std::runtime_error(std::string("Falta atributo ") + attr + " en " + n.name());
    return a.value();
}
double number(pugi::xml_node n, const char* attr) {
    std::istringstream stream(required(n, attr));
    stream.imbue(std::locale::classic());
    double v; stream >> v;
    if (!stream || !stream.eof() || !std::isfinite(v))
        throw std::runtime_error(std::string("Numero invalido: ") + attr);
    return v;
}
int dimension(pugi::xml_node n, const char* attr) {
    double v = number(n, attr);
    if (v < 1 || v > 16384 || std::floor(v) != v)
        throw std::runtime_error("Resolucion invalida (1..16384)");
    return static_cast<int>(v);
}
pugi::xml_node param(pugi::xml_node parent, const char* group, const char* key) {
    for (auto g : parent.children("Params")) {
        if (std::string(g.attribute("name").value()) != group) continue;
        for (auto p : g.children())
            if (std::string(p.attribute("name").value()) == key) return p;
    }
    return {};
}
std::string value(pugi::xml_node parent, const char* group, const char* key) {
    return required(param(parent, group, key), "value");
}
bool flag(pugi::xml_node n, const char* group, const char* key) {
    auto s = value(n, group, key);
    if (s != "0" && s != "1") throw std::runtime_error("BOOL invalido");
    return s == "1";
}
std::vector<glm::vec2> points(pugi::xml_node n, size_t count) {
    std::vector<glm::vec2> result;
    for (auto v : n.children("v")) result.emplace_back(number(v, "x"), number(v, "y"));
    if (result.size() != count) throw std::runtime_error(std::string("Contorno no soportado: ") + n.name());
    return result;
}
bool close(glm::vec2 a, glm::vec2 b) { return glm::distance(a,b) <= 0.01f; }
}
Composition ResolumeXmlLoader::load(const std::string& path) {
    pugi::xml_document doc;
    auto parsed = doc.load_file(path.c_str());
    if (!parsed) throw std::runtime_error(std::string("XML invalido: ") + parsed.description());
    auto root = doc.child("XmlState"), setup = root.child("ScreenSetup");
    if (!setup) throw std::runtime_error("No es un XML de Advanced Output soportado");
    Composition c;
    auto version = root.child("versionInfo");
    c.arenaVersion = required(version,"majorVersion") + "." + required(version,"minorVersion") + "." + required(version,"microVersion");
    auto size = setup.child("CurrentCompositionTextureSize");
    c.width = dimension(size, "width"); c.height = dimension(size, "height");
    std::set<std::string> ids;
    for (auto n : setup.child("screens").children("Screen")) {
        Screen s;
        s.id = required(n,"uniqueId"); s.name = value(n,"Params","Name");
        if (!ids.insert("screen:" + s.id).second) throw std::runtime_error("ID de screen duplicado");
        s.enabled = flag(n,"Params","Enabled"); s.hidden = flag(n,"Params","Hidden");
        auto device = n.child("OutputDevice").child("OutputDeviceVirtual");
        s.width = dimension(device,"width"); s.height = dimension(device,"height");
        for (auto layer : n.child("layers").children()) {
            if (std::string(layer.name()) != "Slice") throw std::runtime_error("Tipo de layer no soportado: " + std::string(layer.name()));
            Slice sl;
            sl.id = required(layer,"uniqueId"); sl.name = value(layer,"Common","Name");
            if (!ids.insert("slice:" + sl.id).second) throw std::runtime_error("ID de slice duplicado");
            sl.enabled = flag(layer,"Common","Enabled");
            sl.inputPoints = points(layer.child("InputRect"),4);
            sl.outputPoints = points(layer.child("OutputRect"),4);
            sl.inputOrientation = static_cast<int>(number(layer.child("InputRect"),"orientation"));
            sl.outputOrientation = static_cast<int>(number(layer.child("OutputRect"),"orientation"));
            auto warp = layer.child("Warper");
            sl.inputSource = value(layer,"Input","Input Source");
            sl.pointMode = value(warp,"Warper","Point Mode");
            auto oflip = value(layer,"Output","Flip"), wflip = value(warp,"Warper","Flip");
            if (oflip != "0" || wflip != "0" || sl.inputOrientation != 0 || sl.outputOrientation != 0)
                throw std::runtime_error(sl.name + ": flip/orientacion aun no verificados");
            auto gridNode = warp.child("BezierWarper");
            if (dimension(gridNode,"controlWidth") != 4 || dimension(gridNode,"controlHeight") != 4 || sl.pointMode != "PM_LINEAR")
                throw std::runtime_error(sl.name + ": modo de warp no soportado");
            sl.warpPoints = points(gridNode.child("vertices"),16);
            auto src = points(warp.child("Homography").child("src"),4);
            auto dst = points(warp.child("Homography").child("dst"),4);
            for (int i=0; i<4; ++i)
                if (!close(src[i], dst[i]) || !close(src[i], sl.outputPoints[i]))
                    throw std::runtime_error(sl.name + ": perspectiva aun no verificada");
            for (int row=0; row<4; ++row) for (int col=0; col<4; ++col) {
                float u=col/3.f, v=row/3.f;
                auto expected = glm::mix(glm::mix(sl.outputPoints[0],sl.outputPoints[1],u),
                                         glm::mix(sl.outputPoints[3],sl.outputPoints[2],u),v);
                if (!close(expected, sl.warpPoints[row*4+col]))
                    throw std::runtime_error(sl.name + ": curva deformada aun no verificada");
            }
            s.slices.push_back(std::move(sl));
        }
        c.screens.push_back(std::move(s));
    }
    if (c.screens.empty()) throw std::runtime_error("El XML no contiene screens");
    return c;
}
}
