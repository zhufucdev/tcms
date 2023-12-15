#include "strings.h"

std::string strings::trim(const std::string &str) {
    int i, j;
    for (i = 0; i < str.length(); ++i) {
        if (str[i] != ' ') {
            break;
        }
    }
    for (j = str.length() - 1; j >= 0; j--) {
        if (str[j] != ' ') {
            break;
        }
    }
    return str.substr(i, j + 1);
}

std::vector<std::string> strings::split(const std::string &str, char separator) {
    std::vector<std::string> buf;
    int i, j;
    for (i = 0, j = 0; i < str.length(); i++) {
        if (str[i] == separator) {
            buf.emplace_back(str.substr(j, i));
            j = i + 1;
        }
    }
    return buf;
}
