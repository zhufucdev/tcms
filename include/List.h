#ifndef TCMS_LIST_H
#define TCMS_LIST_H

#include "Article.h"

namespace tcms {
    namespace behavior {
        class ListArticle {
            const Article *article;
            const bool detailed, separator, dot_name;

        public:
            ListArticle(const Article *article, bool detailed, bool separator = true, bool dot_name = false);

            friend std::ostream &operator<<(std::ostream &os, const ListArticle &lr);
        };

        class ListInRoot {
            const std::vector<Article *> &articles;
            const bool detailed, all;

        public:
            explicit ListInRoot(const std::vector<Article *> &articles, bool detailed, bool all);

            friend std::ostream &operator<<(std::ostream &os, const ListInRoot &lr);
        };

        class ListInArticle {
            const Article *article;
            const bool detailed, all;
            const unsigned char type;

        public:
            explicit ListInArticle(const Article *article, bool detailed, bool all, unsigned char type_filter);

            friend std::ostream &operator<<(std::ostream &os, const ListInArticle &la);
        };

    }
}

#endif //TCMS_LIST_H
