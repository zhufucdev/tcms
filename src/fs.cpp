#include "fs.h"
#include <fstream>
#include <iostream>

using namespace fs;

template<char separator>
inline Path string_to_path_temp(const std::string &str) {
    int i, j;
    Path path;
    for (i = 0, j = 0; i < str.length(); i++) {
        if (str[i] == separator) {
            path.push_back(str.substr(j, i - 1));
            j = i + 1;
        }
    }
    return path;
}

template<char separator>
inline std::string path_to_string_temp(const Path &path) {
    std::string str;
    int i;
    for (i = 0; i < path.size() - 1; ++i) {
        str += path[i];
        str += separator;
    }
    str += path[i];
    return str;
}

#if _WIN32
#include <windows.h>

bool fs::create_directory(const std::string& path) {
    return CreateDirectory(path.c_str(), NULL) != 0;
}

std::string fs::path_to_string(Path path) {
    return path_to_string_temp<'\\'>(path);
}

Path fs::string_to_path(const std::string &str) {
    return string_to_path_temp<'\\'>(str);
}

#include <windows.h>
#include <string>

WindowsFileWalkerIterator(const Path &directory, bool over) : dirpath(directory), over(over) {
    if (!over) {
        dir = FindFirstFile((path_to_string(directory) + "\\*").c_str(), &ffd);
        if (dir == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("directory not found");
        }
    }
}

~WindowsFileWalkerIterator() {
    if (dir != INVALID_HANDLE_VALUE) {
        FindClose(dir);
    }
}

WindowsFileWalkerIterator &operator++() override {
    if (!over) {
        if (!FindNextFile(dir, &ffd)) {
            over = true;
        } else {
            count++;
        }
    }
    return *this;
}

bool operator==(const WindowsFileWalkerIterator &other) const override {
    if (over) {
        return other.over;
    } else {
        return other.count == count;
    }
}

bool operator!=(const WindowsFileWalkerIterator &other) const override {
    return !(*this == other);
}

Path operator*() const override {
    if (over) {
        throw std::runtime_error("dereferencing end iterator");
    }
    Path path{dirpath};
    path.emplace_back(ffd.cFileName);
    return path;
}

FileIteratable<WindowsFileWalterIterator> *fs::list_files(const Path& directory) {
    return FileIterable<WindowsFileWalterIterator>{new WindowsFileWalkerIterator(directory)};
}

#else

#include <sys/stat.h>
#include <dirent.h>

bool fs::create_directory(const std::string &path) {
    return mkdir(path.c_str(), 0777) != -1;
}

std::string fs::path_to_string(Path path) {
    return path_to_string_temp<'/'>(path);
}

Path fs::string_to_path(const std::string &str) {
    return string_to_path_temp<'/'>(str);
}

UnixFileWalkerIterator::UnixFileWalkerIterator(const Path &directory, bool over) : dirpath(directory), over(over) {
    if (!over) {
        auto sp = path_to_string(directory);
        dir = opendir(sp.c_str());
        if (dir == nullptr) {
            throw std::runtime_error("directory not found");
        }
        ent = readdir(dir);
    } else {
        dir = nullptr;
        ent = nullptr;
    }
}

UnixFileWalkerIterator::~UnixFileWalkerIterator() {
    delete ent;
    if (!over) {
        closedir(dir);
    }
}

UnixFileWalkerIterator &UnixFileWalkerIterator::operator++() {
    if (!over) {
        ent = readdir(dir);
        if (ent == nullptr) {
            over = true;
            closedir(dir);
        }
        count++;
    }
    return *this;
}

bool UnixFileWalkerIterator::operator==(const fs::UnixFileWalkerIterator &other) const {
    if (over) {
        return other.over && other.dirpath == dirpath;
    } else if (other.over) {
        return false;
    } else {
        return other.count == count;
    }
}

bool UnixFileWalkerIterator::operator!=(const fs::UnixFileWalkerIterator &other) const {
    return !(*this == other);
}

Path UnixFileWalkerIterator::operator*() const {
    if (ent == nullptr) {
        throw std::runtime_error("dereferencing null pointer");
    }
    Path path{dirpath};
    path.emplace_back(ent->d_name);
    return path;
}

FileIterable<UnixFileWalkerIterator> fs::list_files(const fs::Path &directory) {
    return FileIterable<UnixFileWalkerIterator>{new UnixFileWalkerIterator(directory),
                                                new UnixFileWalkerIterator(directory, true)};
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


