#ifndef TCMS_LIST_H
#define TCMS_LIST_H

#include "Article.h"

namespace tcms {
    namespace behavior {
        class ListInRoot {
            const std::vector<Article *> &articles;
            const bool detailed;

        public:
            explicit ListInRoot(const std::vector<Article *> &articles, bool detailed);

            friend std::ostream &operator<<(std::ostream &os, const ListInRoot &lr);
        };

        class ListInArticle {
            const Article *article;
            const bool detailed;

        public:
            explicit ListInArticle(Article *article, bool detailed);

            friend std::ostream &operator<<(std::ostream &os, const ListInArticle &la);
        };

    }
}

#endif //TCMS_LIST_H
