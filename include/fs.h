#ifndef TCMS_FS_H
#define TCMS_FS_H

#include <string>
#include <vector>
#include "bytes.h"

namespace fs {
    typedef std::vector<std::string> Path;

    bool create_directory(const std::string &path);

    bool copy(const Path &src, const Path &dst);

    std::string get_extension(const Path &path);

    std::string path_to_string(const Path &path);

    Path string_to_path(const std::string &str);

    std::string get_name(const Path &path);

    std::string get_name_without_extension(const Path &path);

    bool is_hidden(const Path &path);

    void write_file(const Path &dst, const ByteArray &src);

    ByteArray read_file(const Path &src);

    bool remove_file(const Path &path);

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

    class FileAssociated : public bytes::BinarySerializable {
    public:
        virtual Path get_path() const = 0;

        virtual void write_to_file();

        virtual void remove();
    };

#if _WIN32
#include <windows.h>
    class WindowsFileWalkerIterator : public FileIterator {
        Path dirpath;
        HANDLE dir;
        WIN32_FIND_DATA ffd;
        bool over = false;
        int count = 0;
    public:
        WindowsFileWalkerIterator(const Path& directory, bool over = false);
        ~WindowsFileWalkerIterator();
        WindowsFileWalkerIterator& operator++();
        bool operator==(const WindowsFileWalkerIterator& other) const;
        bool operator!=(const WindowsFileWalkerIterator& other) const;
        Path operator*() const;
    };

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