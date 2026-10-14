#pragma once
#include <filesystem>
#include <string>
#include <sstream>
namespace sb {
// Wait for exports to settle; a partial write must never replace the live mapping.
class FileWatcher {
public:
    void reset(const std::string& file){path=file;seen=signature();pending=false;}
    bool poll(double now){
        auto current=signature();
        if(current!=seen){seen=current;changedAt=now;pending=true;}
        if(pending && now-changedAt>=0.35){pending=false;return true;}return false;
    }
private:
    std::string path,seen;double changedAt=0;bool pending=false;
    std::string signature()const{
        namespace fs=std::filesystem;std::ostringstream out;std::error_code ec;
        auto stamp=[&](const fs::path& p){auto t=fs::last_write_time(p,ec);if(ec){out<<"missing";ec.clear();return;}out<<p.filename().string()<<t.time_since_epoch().count()<<fs::file_size(p,ec);ec.clear();};
        if(path.empty())return "";
        if(fs::is_directory(path,ec)){for(auto& item:fs::directory_iterator(path,ec))if(item.is_regular_file())stamp(item.path());}else stamp(path);
        return out.str();
    }
};
}
