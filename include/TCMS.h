#ifndef TCMS_H
#define TCMS_H

#include "Article.h"

namespace tcms {
    class TCMS {
        bool running;
        std::vector<Article *> articles;

        bool change_work(Article *article);

        bool change_work(Frame *frame);

        bool change_work(Contact *contact);

    public:
        TCMS();

        ~TCMS();

        void event_loop();

        void interrupt(int signal);

        bool new_article(const std::string &name);

        bool delete_article(const std::string &name);

        Article *find_article(const std::string &name);


    };
}

#endif