#include "terminal.h"
#include "strings.h"
#include <iostream>

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

terminal::ReadName terminal::read_name(const std::vector<std::string> &args, terminal::str_vec_size_t offset) {
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

std::string terminal::read_paragraph(const std::vector<std::string> &args, terminal::str_vec_size_t offset) {
    return strings::join(args.begin() + offset, args.end(), ' ');
}
