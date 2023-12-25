#include "strhelper.h"
#include <regex>
#include <cctype>
#include <algorithm>

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

std::vector<std::string>
strings::split(const std::string &str, const std::string &separators, std::vector<std::string>::size_type limit) {
    std::vector<std::string> buf;
    std::vector<std::string>::size_type i, j;
    for (i = 0, j = 0; i < str.length(); i++) {
        auto separated = false;
        for (const auto &s: separators) {
            if (str[i] == s) {
                separated = true;
                break;
            }
        }
        if (separated) {
            if (limit > 0 && buf.size() >= limit - 1) {
                i = str.length();
                break;
            } else {
                buf.emplace_back(str.substr(j, i - j));
                j = i + 1;
            }
        }
    }
    if (i > j) {
        buf.emplace_back(str.substr(j, i - j));
    }
    return buf;
}

std::string
strings::join(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last,
              char separator) {
    if (first == last) {
        return "";
    }
    std::string str;
    while (first != last) {
        str += *first;
        str += separator;
        first++;
    }
    return str.substr(0, str.length() - 1);
}

std::string strings::truncate(const std::string &str, std::string::size_type cut) {
    if (str.length() > cut) {
        return str.substr(0, cut - 1) + "...";
    } else {
        return str;
    }
}

template<typename charT>
struct my_equal {
    explicit my_equal(const std::locale &loc) : loc_(loc) {}

    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }

private:
    const std::locale &loc_;
};

template<typename T>
bool ci_find_substr(const T &str1, const T &str2, const std::locale &loc = std::locale()) {
    typename T::const_iterator it = std::search(str1.begin(), str1.end(),
                                                str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));
    return it != str1.end();
}

bool strings::match(const std::string &src, const std::string &keyword, bool ignoring_case, bool regex) {
    if (regex) {
        auto flags = std::regex_constants::ECMAScript;
        if (ignoring_case) {
            flags |= std::regex_constants::icase;
        }
        auto r = std::regex(keyword, flags);
        return std::regex_search(src, r);
    } else if (ignoring_case) {
        return ci_find_substr(src, keyword);
    } else {
        return src.find(keyword) != std::string::npos;
    }
}
