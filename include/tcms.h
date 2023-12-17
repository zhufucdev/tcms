#ifndef TCMS_H
#define TCMS_H

#include "content.h"
#include "metadata.h"

namespace tcms {
    class TCMS {
        bool running;
    public:
        TCMS();
        void event_loop();
        void interrupt(int signal);
    };
}

#endif