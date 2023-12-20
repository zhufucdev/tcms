#include "List.h"
#include <ostream>

using namespace tcms::behavior;

ListInRoot::ListInRoot(const std::vector<Article *> &articles, bool detailed)
        : articles(articles), detailed(detailed) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInRoot &lr) {
    if (lr.detailed) {
        for (int i = 0; i < lr.articles.size(); ++i) {
            auto a = lr.articles[i];
            os << a->get_id() << '\t' << a->get_name() << '\t' << a->get_frames().size() << "f" << '\t'
               << fs::path_to_string(a->get_path());
            if (i < lr.articles.size() - 1) {
                os << '\n';
            }
        }
    } else {
        for (auto a: lr.articles) {
            os << a->get_name() << '\t';
        }
    }
    return os;
}

ListInArticle::ListInArticle(tcms::Article *article, bool detailed)
        : article(article), detailed(detailed) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInArticle &la) {
    auto frames = la.article->get_frames();
    if (la.detailed) {
        for (int i = 0; i < frames.size(); ++i) {
            os << frames[i]->get_id() << '\t';
            switch (frames[i]->get_type()) {
                case TITLE:
                    os << "header";
                    break;
                case PARAGRAPH:
                    os << "para";
                    break;
                case IMAGE:
                    os << "image";
                    break;
            }
            os << '\t' << frames[i]->estimated_size() << "B" << '\t'
               << fs::path_to_string(frames[i]->get_path());
            if (i < frames.size() - 1) {
                os << '\n';
            }
        }
    } else {
        for (auto f: frames) {
            os << f->get_id() << '\t';
        }
    }
    return os;
}

