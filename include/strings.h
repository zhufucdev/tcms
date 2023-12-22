//
// Created by Steve Reed on 2023/12/15.
//

#ifndef TCMS_STRINGS_H
#define TCMS_STRINGS_H

#include <string>
#include <vector>

namespace strings {
    std::string trim(const std::string &str);

    std::vector<std::string> split(const std::string &str, char separator, std::vector<std::string>::size_type limit = 0);

    std::string join(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last, char separator = ' ');

    std::string truncate(const std::string &str, std::string::size_type cut = 8);

    template<typename T>
    T parse_number(const std::string &src) {
        T num = 0;
        for (int i = 0; i < src.length(); ++i) {
            num *= 10;
            num += src[i] - '0';
        }
        return num;
    }

    bool match(const std::string& src, const std::string& keyword, bool ignoring_case = false, bool regex = false);
}

#endif //TCMS_STRINGS_H
