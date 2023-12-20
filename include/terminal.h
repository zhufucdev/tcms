#ifndef TCMS_TERMINAL_H
#define TCMS_TERMINAL_H

#include <vector>
#include <string>
#include <map>

namespace terminal {
    typedef std::vector<std::string>::size_type str_vec_size_t;

    struct ReadName {
        str_vec_size_t epos;
        std::string name;
    };

    struct ReadFlags {
        std::string singles;
        std::map<char, std::string> parameters;
    };

    void clear_screen();

    ReadName read_name(const std::vector<std::string>& args, str_vec_size_t offset = 1);

    std::string read_paragraph(const std::vector<std::string>& args, str_vec_size_t offset = 1);

    ReadFlags read_flags(const std::vector<std::string>& args, str_vec_size_t offset = 1);
}

#endif //TCMS_TERMINAL_H
