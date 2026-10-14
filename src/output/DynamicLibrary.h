#pragma once
#include "ofMain.h"
#ifdef TARGET_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace sb {
class DynamicLibrary {
public:
    ~DynamicLibrary(){close();}
    void open(const std::string& path){
        close();
#ifdef TARGET_WIN32
        handle=LoadLibraryA(path.c_str());
#else
        handle=dlopen(path.c_str(),RTLD_NOW|RTLD_LOCAL);
#endif
        if(!handle)throw std::runtime_error("No se pudo cargar biblioteca: "+path);
    }
    template<typename T> T symbol(const char* name){
#ifdef TARGET_WIN32
        auto fn=reinterpret_cast<T>(GetProcAddress(handle,name));
#else
        auto fn=reinterpret_cast<T>(dlsym(handle,name));
#endif
        if(!fn)throw std::runtime_error("Funcion no disponible: "+std::string(name));return fn;
    }
    void close(){
        if(!handle)return;
#ifdef TARGET_WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        handle=nullptr;
    }
private:
#ifdef TARGET_WIN32
    HMODULE handle=nullptr;
#else
    void* handle=nullptr;
#endif
};
}
