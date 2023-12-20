#ifndef TCMS_H
#define TCMS_H

#include "Article.h"

namespace tcms {
    class TCMS {
        bool running;
        std::vector<Article *> articles;

        bool new_article(const std::string &name);

        bool delete_article(const std::string &name);

    public:
        TCMS();

        ~TCMS();

        void event_loop();

        void interrupt(int signal);
    };
}

#endif