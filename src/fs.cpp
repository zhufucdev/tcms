#include "fs.h"
#include <fstream>

bool fs::copy(const std::string &src, const std::string &dst) {
    std::ifstream ss(src, std::ios::binary);
    std::ofstream ds(dst, std::ios::binary | std::ios::trunc);
    ds << ss.rdbuf();
    ds.close();
    return ds.good();
}

std::string fs::get_extension(const std::string &path) {
    for (auto i = path.length() - 1; i >= 0; --i) {
        if (path[i] == '.') {
            return path.substr(i);
        }
    }
    return ".";
}

#if _WIN32
#include <windows.h>

bool fs::create_directory(const std::string& path) {
    return CreateDirectory(path.c_str(), NULL) != 0;
}
#else

#include <sys/stat.h>

bool fs::create_directory(const std::string &path) {
    return mkdir(path.c_str(), 0777) != -1;
}

#endif
