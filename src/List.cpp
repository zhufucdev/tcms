#include "List.h"
#include <ostream>

using namespace tcms::behavior;

ListArticle::ListArticle(const tcms::Article *article, bool detailed, bool separator, bool dot_name)
        : article(article), detailed(detailed), separator(separator), dot_name(dot_name) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListArticle &la) {
    if (la.detailed) {
        os << la.article->get_id() << '\t' << (la.dot_name ? "." : la.article->get_name()) << '\t'
           << la.article->get_frames().size() << "f" << '\t' << fs::path_to_string(la.article->get_path());
        if (la.separator) {
            os << '\n';
        }
    } else {
        os << (la.dot_name ? "." : la.article->get_name());
        if (la.separator) {
            os << '\t';
        }
    }
    return os;
}

ListInRoot::ListInRoot(const std::vector<Article *> &articles, bool detailed, bool all)
        : articles(articles), detailed(detailed), all(all) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInRoot &lr) {
    if (lr.detailed) {
        if (lr.all) {
            os << ".\ttcms_root" << std::endl;
        }
        for (int i = 0; i < lr.articles.size(); ++i) {
            os << ListArticle(lr.articles[i], true, i < lr.articles.size() - 1);
        }
    } else {
        if (lr.all) {
            os << "." << '\t';
        }
        for (const auto &a: lr.articles) {
            os << ListArticle(a, false);
        }
    }
    return os;
}

ListInArticle::ListInArticle(const tcms::Article *article, bool detailed, bool all, unsigned char type_filter)
        : article(article), detailed(detailed), all(all), type(type_filter) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInArticle &la) {
    auto frames = la.article->get_frames();
    if (la.all) {
        os << ListArticle(la.article, la.detailed, true, true);
    }
    if (la.detailed) {
        for (int i = 0; i < frames.size(); ++i) {
            os << frames[i]->get_id() << '\t';
            auto type = frames[i]->get_type();
            if ((type & la.type) != type) {
                continue;
            }
            switch (type) {
                case TITLE:
                    os << "header";
                    break;
                case PARAGRAPH:
                    os << "para";
                    break;
                case IMAGE:
                    os << "image";
                    break;
                default:
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
            if ((f->get_type() & la.type) != f->get_type()) {
                continue;
            }
            os << f->get_id() << '\t';
        }
    }
    return os;
}
