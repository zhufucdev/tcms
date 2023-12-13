#ifndef TCMS_FS_H
#define TCMS_FS_H

#include <string>

namespace fs {
    bool create_directory(const std::string& path);
    bool copy(const std::string &src, const std::string &dst);
    std::string get_extension(const std::string &path);
}

#endif