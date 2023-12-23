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

    class ReadFlags {
        std::map<char, bool> singles;
        std::map<std::string, bool> named;
        std::map<char, std::string> parameters;
    public:
        const str_vec_size_t epos;

        ReadFlags(const std::map<char, bool> &singles, const std::map<std::string, bool> &named,
                  const std::map<char, std::string> &parameters, str_vec_size_t epos);

        bool has_single(char flag);

        bool has_named(std::string name);

        std::string get_parameter(char flag, const std::string &def);
    };

    void clear_screen();

    inline bool is_flag(const std::string &arg) {
        return arg.length() >= 2 && arg[0] == '-';
    }

    std::vector<std::string> read_args(const std::string &str);

    ReadName read_name(const std::vector<std::string> &args, str_vec_size_t offset = 1);

    std::string read_paragraph(const std::vector<std::string> &args, str_vec_size_t offset = 1);

    ReadFlags read_flags(const std::vector<std::string> &args, str_vec_size_t offset = 1);
}

#endif //TCMS_TERMINAL_H
