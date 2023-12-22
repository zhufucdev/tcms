#include "tcms.h"
#include <ostream>

using namespace tcms::behavior;

Behavior::Behavior(const tcms::Context &ctx) : ctx(ctx) {}

ListArticle::ListArticle(const tcms::Context &ctx, const tcms::Article *article, bool detailed, bool separator,
                         bool dot_name)
        : Behavior(ctx), article(article), detailed(detailed), separator(separator), dot_name(dot_name) {}

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

ListInRoot::ListInRoot(const tcms::Context &ctx, bool detailed, bool all)
        : Behavior(ctx), detailed(detailed), all(all) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInRoot &lr) {
    if (lr.detailed) {
        if (lr.all) {
            os << ".\ttcms_root" << std::endl;
        }
        for (int i = 0; i < lr.ctx.articles.size(); ++i) {
            os << ListArticle(lr.ctx, lr.ctx.articles[i], true, i < lr.ctx.articles.size() - 1);
        }
    } else {
        if (lr.all) {
            os << "." << '\t';
        }
        for (const auto &a: lr.ctx.articles) {
            os << ListArticle(lr.ctx, a, false);
        }
    }
    return os;
}

ListInArticle::ListInArticle(const Context &ctx, const tcms::Article *article, bool detailed, bool all,
                             unsigned char type_filter)
        : Behavior(ctx), article(article), detailed(detailed), all(all), type(type_filter) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInArticle &la) {
    auto frames = la.article->get_frames();
    if (la.all) {
        os << ListArticle(la.ctx, la.article, la.detailed, true, true);
        os << ListMetadata(la.ctx, const_cast<Article *>(la.article)->get_metadata(), la.detailed, false);
    }
    if (la.detailed) {
        for (int i = 0; i < frames.size(); ++i) {
            os << frames[i]->get_id() << '\t';
            auto type = frames[i]->get_type();
            if ((type & la.type) != type) {
                continue;
            }
            switch (type) {
                case HEADER:
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

ListMetadata::ListMetadata(const tcms::Context &ctx, const tcms::Metadata &metadata, bool detailed, bool dot_name)
        : Behavior(ctx), metadata(metadata), detailed(detailed), dot_name(dot_name) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const tcms::behavior::ListMetadata &lm) {
    if (lm.detailed) {
        os << "nid\t";
    }
    if (lm.dot_name) {
        os << ".";
    } else {
        os << ".metadata";
    }
    if (lm.detailed) {
        auto tags = lm.metadata.get_tags().size();
        os << '\t' << tags << " tag";
        if (tags > 1) {
            os << 's';
        }
        os << '\n';
    } else {
        os << '\t';
    }
    return os;
}

ListInMetadata::ListInMetadata(const tcms::Context &ctx, const tcms::Metadata &metadata, bool detailed, bool all,
                               unsigned char type_filer)
        : Behavior(ctx), metadata(metadata), detailed(detailed), all(all), type(type_filer) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const tcms::behavior::ListInMetadata &la) {
    if (la.all) {
        os << ListMetadata(la.ctx, la.metadata, la.detailed, true);
    }
    auto tags = la.metadata.get_tags();
    if (la.detailed) {
        for (int i = 0; i < tags.size(); i++) {
            os << i << '\t';
            switch (tags[i]->get_type()) {
                case LANG:
                    os << "lang" << '\t' << tags[i]->to_string();
                    break;
                case AUTHOR:
                    os << "author" << '\t' << tags[i]->to_string() << '\t'
                       << fs::path_to_string(dynamic_cast<AuthorTag *>(tags[i])->get_author()->get_path());
                    break;
                case TITLE:
                    os << "title\t" << '\t' << tags[i]->to_string();
                    break;
            }
            if (i < tags.size() - 1) {
                os << '\n';
            }
        }
    } else {
        for (int i = 0; i < tags.size(); ++i) {
            os << i << '\t';
        }
    }
    return os;
}

ListInElement::ListInElement(const tcms::Context &ctx, const tcms::Element *ele, bool detailed, bool all,
                             unsigned char type_filter)
        : Behavior(ctx), element(ele), detailed(detailed), all(all), type(type_filter) {}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const ListInElement &le) {
    switch (le.element->get_type()) {
        case ROOT:
            os << ListInRoot(le.ctx, le.detailed, le.all);
            break;
        case ARTICLE:
            os << ListInArticle(le.ctx, dynamic_cast<const ArticleElement *>(le.element)->get(), le.detailed, le.all,
                                le.type);
            break;
        case FRAME:
            os << "." << std::endl;
            break;
        case CONTACT:
            os << "idk" << std::endl;
            break;
        case METADATA:
            os << ListInMetadata(le.ctx, dynamic_cast<const MetadataElement *>(le.element)->get(), le.detailed, le.all,
                                 le.type);
            break;
        case TAG:
            break;
    }
    return os;
}
