#include "tcms.h"
#include "strings.h"
#include <ostream>

namespace tcms {
    Element::Element() : parent(nullptr) {}

    RootElement::RootElement(tcms::Context &ctx) : ctx(ctx) {}

    ElementType RootElement::get_type() const {
        return ROOT;
    }

    bool RootElement::operator==(const tcms::Element *other) {
        if (auto _ = dynamic_cast<const RootElement *>(other)) {
            return true;
        }
        return false;
    }

    Element *RootElement::resolve(const std::string &path) {
        auto split = strings::split(path, '/', 2);
        if (split.empty() || split.size() == 1 && split[0] == ".") {
            return this;
        }

        auto article_name = split[0];
        auto iter = std::find_if(ctx.articles.begin(), ctx.articles.end(),
                                 [&](auto a) { return a->get_name() == article_name; });
        if (iter == ctx.articles.end()) {
            return nullptr;
        } else {
            auto a = new ArticleElement(*iter, ctx);
            if (split.size() == 2) {
                return a->resolve(split[1]);
            } else {
                return a;
            }
        }
    }

    void RootElement::remove() {
        throw std::runtime_error("removing tcms root");
    }

    void RootElement::output(std::ostream &os, ExportVariant variant) {
        os << "tcms" << std::endl;
    }

    ArticleElement::ArticleElement(tcms::Article *article, Context &ctx)
            : article(article), ctx(ctx) {}

    ArticleElement::ArticleElement(const std::string &name, tcms::Context &ctx) : ctx(ctx) {
        auto ele = ctx.get_current_working_element()->resolve(name);
        if (ele != nullptr) {
            throw std::runtime_error("duplicated name: " + name);
        }

        auto a = new Article(name);
        article = a;
        ctx.articles.push_back(a);
        a->write_to_file();
    }

    Article *ArticleElement::get() const {
        return article;
    }

    ElementType ArticleElement::get_type() const {
        return ElementType::ARTICLE;
    }

    void ArticleElement::remove() {
        ctx.articles.erase(std::find(ctx.articles.begin(), ctx.articles.end(), article));
        article->remove();
        delete article;
    }

    bool ArticleElement::operator==(const tcms::Element *other) {
        if (auto a = dynamic_cast<const ArticleElement *>(other)) {
            return *(a->article) == *article;
        }
        return false;
    }

    Element *ArticleElement::resolve(const std::string &path) {
        auto split = strings::split(path, '/', 2);
        if (split.empty() || split[0] == ".") {
            return this;
        } else if (split[0] == ".." && parent != nullptr) {
            return parent->resolve(split.size() == 2 ? split[1] : "");
        } else {
            auto frames = article->get_frames();
            auto id = strings::parse_number<id_type>(split[0]);
            auto iter = std::find_if(frames.begin(), frames.end(), [&](auto e) { return e->get_id() == id; });
            if (iter != frames.end()) {
                auto ele = new FrameElement(*iter, ctx);
                ele->parent = this;
                return ele;
            }
        }
        return nullptr;
    }

    void ArticleElement::output(std::ostream &os, tcms::ExportVariant variant) {
        switch (variant) {
            case PLAIN:
                os << behavior::PlainArticle(ctx, get());
                break;
            case MARKDOWN:
                os << behavior::MarkdownArticle(ctx, get());
                break;
            case HTML:
                break;
        }
    }

    FrameElement::FrameElement(tcms::FrameGetter *getter, tcms::Context &ctx) : getter(getter), ctx(ctx) {}

    ElementType FrameElement::get_type() const {
        return FRAME;
    }

    Element *FrameElement::resolve(const std::string &path) {
        if (path == ".") {
            return this;
        } else if (path == "..") {
            return parent;
        } else {
            return nullptr;
        }
    }

    bool FrameElement::operator==(const tcms::Element *other) {
        if (auto f = dynamic_cast<const FrameElement *>(other)) {
            return f->getter->get_id() == getter->get_id();
        }
        return false;
    }

    FrameGetter *FrameElement::get() const {
        return getter;
    }

    void FrameElement::remove() {
        if (auto a = dynamic_cast<ArticleElement *>(parent)) {
            a->get()->remove_frame(getter);
        }
    }

    void FrameElement::output(std::ostream &os, tcms::ExportVariant variant) {
        switch (variant) {
            case PLAIN:
                os << behavior::PlainFrameElement(ctx, this);
                break;
            case MARKDOWN:
                os << behavior::MarkdownFrameElement(ctx, this);
                break;
            case HTML:
                break;
        }
    }
}