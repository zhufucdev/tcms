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

std::vector<std::string> strings::split(const std::string &str, char separator, std::vector<std::string>::size_type limit) {
    std::vector<std::string> buf;
    int i, j;
    for (i = 0, j = 0; i < str.length(); i++) {
        if (str[i] == separator) {
            buf.emplace_back(str.substr(j, i - j));
            j = i + 1;
        }
        if (limit > 0 && buf.size() >= limit) {
            break;
        }
    }
    if (i > j) {
        buf.emplace_back(str.substr(j, i));
    }
    return buf;
}

std::string
strings::join(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last,
              char separator) {
    std::string str;
    while (first != last) {
        str += *first;
        str += separator;
        first++;
    }
    return std::string{str.c_str(), str.length() - 1};
}

std::string strings::truncate(const std::string &str, std::string::size_type cut) {
    if (str.length() > cut) {
        return str.substr(0, cut - 1) + "…";
    } else {
        return str;
    }
}
