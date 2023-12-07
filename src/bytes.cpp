#include <bytes.h>


ByteArray ByteArray::operator+(size_t offset) const {
    return {content + offset, len - offset};
}

ByteArray ByteArray::operator-(size_t offset) const {
    return {content - offset, len + offset};
}

std::vector<std::string> bytes::to_string(const ByteArray &ba) {
    int i = 0, j = 0;
    std::vector<std::string> heap;
    while (i < ba.len) {
        if (ba.content[i] == 0) {
            std::string substr(ba.content + j, i - j);
            heap.push_back(substr);
            j = i + 1;
        }
        i++;
    }
    if (j != i) {
        std::string substr(ba.content + j, ba.len - j);
        heap.push_back(substr);
    }
    return heap;
}
