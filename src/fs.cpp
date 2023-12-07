#include "fs.h"

#if _WIN32
#include <windows.h>

void fs::create_directory(const std::string& path) {
    CreateDirectory(path.c_str(), NULL);
}
#else

#include <sys/stat.h>

bool fs::create_directory(const std::string &path) {
    return mkdir(path.c_str(), 0777) != -1;
}

#endif
