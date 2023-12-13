#ifndef TCMS_FS_H
#define TCMS_FS_H

#include <string>
#include <vector>

typedef std::vector<std::string> Path;
#define Path(x) Path{x}

namespace fs {
    bool create_directory(const std::string& path);
    bool copy(const Path &src, const Path &dst);
    std::string get_extension(const Path &path);
    std::string path_to_string(Path path);
}

#endif