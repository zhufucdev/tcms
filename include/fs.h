#ifndef TCMS_FS_H
#define TCMS_FS_H

#include <string>
#include <vector>

namespace fs {
    typedef std::vector<std::string> Path;

    bool create_directory(const std::string &path);

    bool copy(const Path &src, const Path &dst);

    std::string get_extension(const Path &path);

    std::string path_to_string(Path path);

    Path string_to_path(const std::string &str);

    typedef std::iterator<std::input_iterator_tag, Path, int, const fs::Path *, Path> FileIterator;

    template<class Iterator>
    class FileIterable {
        Iterator *b;
        Iterator *e;
    public:
        explicit FileIterable(Iterator *begin, Iterator *end) : b(begin), e(end) {}

        Iterator &begin() const { return *b; }

        Iterator &end() const { return *e; }
    };

#if _WIN32
    class WindowsFileWalkerIterator : FileIterator {
        Path dirpath;
        HANDLE dir;
        WIN32_FIND_DATA ffd;
        bool over = false;
        int count = 0;
    public:
        WindowsFileWalkerIterator(const Path &directory, bool over = false);
        ~WindowsFileWalkerIterator();
        bool operator==(const WindowsFileWalkerIterator &other) const override;
        bool operator!=(const WindowsFileWalkerIterator &other) const override;
        Path operator*() const override;
    }
    FileIterable<WindowsFileWalkerIterator> list_files(const Path &directory);
#else

#include "dirent.h"

    class UnixFileWalkerIterator : public FileIterator {
        Path dirpath;
        DIR *dir;
        struct dirent *ent = nullptr;
        bool over = false;
        int count = 0;
    public:
        explicit UnixFileWalkerIterator(const Path &directory, bool over = false);

        ~UnixFileWalkerIterator();

        UnixFileWalkerIterator &operator++();

        bool operator==(const UnixFileWalkerIterator &other) const;

        bool operator!=(const UnixFileWalkerIterator &other) const;

        reference operator*() const;
    };

    FileIterable<UnixFileWalkerIterator> list_files(const Path &directory);

#endif
}

#endif