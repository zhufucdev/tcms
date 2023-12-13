#ifndef TCMS_BYTES_H
#define TCMS_BYTES_H
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

struct ByteArray {
public:
    char *content;
    size_t len;

    ByteArray operator+(size_t offset) const;
    ByteArray operator-(size_t offset) const;
};

namespace bytes {
    class BinarySerializable {
        virtual ByteArray serialize() const = 0;
    };

    template<typename T>
    size_t write_number(char *dst, T count) {
        size_t len = sizeof(T);
        int n = len - sizeof(count);
        while (n < len) {
            dst[n] = (count >> (sizeof(count) - n - 1) * 8);
            n++;
        }
        return len;
    }

    template<typename T>
    T read_number(const char *src) {
        int n = 0;
        size_t msize = 0, len = sizeof(T);
        while (n < len) {
            msize <<= sizeof(char);
            msize |= src[n++];
        }
        return msize;
    }

    template<typename T>
    size_t concat_len(const T &str) { return str.length() + 1; }

    template<typename T, typename... Args>
    size_t concat_len(const T &first, const Args &...args) {
        return first.length() + 1 + concat_len(args...);
    }

    template<typename T>
    void append_bytes_buffer(char *start, const T &content) {
        std::strcpy(start, content.c_str());
    }

    template<typename T, typename... Args>
    void append_bytes_buffer(char *start, const T &next, const Args &...args) {
        append_bytes_buffer(start, next);
        append_bytes_buffer(start + next.length() + 1, args...);
    }

    template<typename... Args>
    ByteArray from_string(const std::string &first, const Args &...args) {
        size_t len = concat_len(first, args...);
        char *buf = (char *) calloc(len, sizeof(char));
        append_bytes_buffer(buf, first, args...);
        return {buf, len};
    }

    std::vector<std::string> to_string(const ByteArray &ba);
}
#endif