#include "TCMS.h"
#include <csignal>

using namespace tcms;

std::map<id_type, size_t> FrameGetter::rc{};
std::map<id_type, tcms::Frame *> FrameGetter::cache{};

std::map<id_type, size_t> ContactGetter::rc{};
std::map<id_type, Contact *> ContactGetter::cache{};

TCMS client;

void interrupt(int s) {
    client.interrupt(s);
}

int main() {
    signal(SIGINT, interrupt);
    client.event_loop();
    return 0;
}
