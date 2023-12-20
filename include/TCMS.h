#ifndef TCMS_H
#define TCMS_H

#include "Article.h"

namespace tcms {
    class TCMS {
        bool running;
        std::vector<Article *> articles;
    public:
        TCMS();
        void event_loop();
        void interrupt(int signal);
    };
}

#endif