#include "fs.h"
#include <fstream>
#include <numeric>

#if _WIN32
#include <windows.h>

bool fs::create_directory(const std::string& path) {
    return CreateDirectory(path.c_str(), NULL) != 0;
}

std::string fs::path_to_string(Path path) {
    return std::accumulate(path.begin(), path.end(), std::string(""), [&](auto p, auto c) { return p + "\\" + c; });
}
#else

#include <sys/stat.h>

bool fs::create_directory(const std::string &path) {
    return mkdir(path.c_str(), 0777) != -1;
}

std::string fs::path_to_string(Path path) {
    return std::accumulate(path.begin(), path.end(), std::string(""), [&](auto p, auto c) { return p + "/" + c; });
}

#endif

bool fs::copy(const Path &src, const Path &dst) {
    std::ifstream ss(path_to_string(src), std::ios::binary);
    std::ofstream ds(path_to_string(dst), std::ios::binary | std::ios::trunc);
    ds << ss.rdbuf();
    ds.close();
    return ds.good();
}

std::string fs::get_extension(const Path &path) {
    auto tail = path[path.size() - 1];
    for (int i = tail.length() - 1; i >= 0; --i) {
        if (tail[i] == '.') {
            return tail.substr(i);
        }
    }
    return ".";
}

