#include "fs.h"

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
