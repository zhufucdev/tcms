//
// Created by Steve Reed on 2023/12/15.
//

#ifndef TCMS_STRINGS_H
#define TCMS_STRINGS_H

#include <string>
#include <vector>

namespace strings {
    std::string trim(const std::string &str);
    std::vector<std::string> split(const std::string &str, char separator);
}

#endif //TCMS_STRINGS_H
