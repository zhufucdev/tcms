#include "terminal.h"
#include "strings.h"
#include <iostream>

using namespace terminal;

#if _WIN32

#include <windows.h>

void terminal::clear_screen() {
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}

#else

void terminal::clear_screen() {
    std::cout << "\x1B[2J\x1B[H";
}

#endif

ReadName terminal::read_name(const std::vector<std::string> &args, str_vec_size_t offset) {
    str_vec_size_t i = offset;
    if (i >= args.size()) {
        return {i, ""};
    }
    std::string name;
    while (true) {
        name += args[i];
        if (args[i][args[i].length() - 1] != '\\' || i >= args.size()) {
            break;
        }
        name = name.substr(0, name.length() - 1) + " ";
        i++;
    }
    return {i + 1, name};
}

std::string terminal::read_paragraph(const std::vector<std::string> &args, str_vec_size_t offset) {
    return strings::join(args.begin() + offset, args.end(), ' ');
}

inline bool is_flag(const std::string &arg) {
    return arg.length() >= 2 && arg[0] == '-';
}

ReadFlags terminal::read_flags(const std::vector<std::string> &args, str_vec_size_t offset) {
    std::map<char, bool> singles;
    std::map<std::string, bool> named;
    std::map<char, std::string> parameters;
    auto read_n = read_name(args, offset);
    while (true) {
        if (is_flag(read_n.name)) {
            auto is_named = false;
            if (read_n.name.length() == 2 && read_n.epos < args.size()) {
                auto flag = read_n.name[1];
                read_n = read_name(args, read_n.epos);
                if (!is_flag(read_n.name)) {
                    parameters[flag] = read_n.name;
                    continue;
                } else {
                    read_n.name = std::string{'-', flag};
                }
            } else if (read_n.name.length() > 2 && read_n.name.substr(0, 2) == "--") {
                is_named = true;
            }
            if (!is_named) {
                for (auto c: read_n.name.substr(1)) {
                    singles[c] = true;
                }
            } else {
                named[read_n.name.substr(2)] = true;
            }
        }
        if (read_n.epos >= args.size()) {
            break;
        }
        read_n = read_name(args, read_n.epos);
    }
    return {singles, named, parameters};
}

ReadFlags::ReadFlags(const std::map<char, bool> &singles, const std::map<std::string, bool> &named,
                     const std::map<char, std::string> &parameters)
        : singles(singles), named(named), parameters(parameters) {}

bool ReadFlags::has_single(char flag) {
    return singles[flag];
}

bool ReadFlags::has_named(std::string name) {
    return named[name];
}

std::string ReadFlags::get_parameter(char flag, const std::string &def) {
    if (parameters.find(flag) != parameters.end()) {
        return parameters[flag];
    } else {
        return def;
    }
}
