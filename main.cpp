#include "TCMS.h"
#include <csignal>

using namespace tcms;

TCMS client;

void interrupt(int s) {
    client.interrupt(s);
}

int main() {
    signal(SIGINT, interrupt);
    client.event_loop();
    return 0;
}
