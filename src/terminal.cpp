#include "terminal.h"
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
