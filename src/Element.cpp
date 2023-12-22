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
            a->parent = this;
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
        if (article == nullptr) {
            throw std::runtime_error("error while getting article (null pointer)");
        }
        return article;
    }

    ElementType ArticleElement::get_type() const {
        return ElementType::ARTICLE;
    }

    void ArticleElement::remove() {
        ctx.articles.erase(std::find(ctx.articles.begin(), ctx.articles.end(), article));
        get()->remove();
        delete article;
        article = nullptr;
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
            return parent->resolve(split.size() == 2 ? split[1] : ".");
        } else if (split[0] == ".metadata") {
            auto ele = new MetadataElement(article->get_metadata(), ctx);
            ele->parent = this;
            return ele->resolve(split.size() == 2 ? split[1] : ".");
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

    void html_os_prefix(std::ostream &os, const std::string &title) {
        os << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>)" << title << R"(</title>
</head>
<body>
)";
    }

    void html_os_suffix(std::ostream &os) {
        os << R"(</body>
</html>
)";
    }

    void ArticleElement::output(std::ostream &os, tcms::ExportVariant variant) {
        switch (variant) {
            case PLAIN:
                for (auto f: get()->get_frames()) {
                    auto e = FrameElement(f, ctx);
                    os << PlainFrameElement(e) << '\n';
                }
                break;
            case MARKDOWN:
                for (auto f: get()->get_frames()) {
                    auto e = FrameElement(f, ctx);
                    os << MarkdownFrameElement(e) << '\n';
                }
                break;
            case HTML:
                html_os_prefix(os, get()->get_name());
                for (auto f: get()->get_frames()) {
                    auto e = FrameElement(f, ctx);
                    os << HTMLFrameElement(e) << '\n';
                }
                html_os_suffix(os);
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
        if (getter == nullptr) {
            throw std::runtime_error("error while getting frame element (null pointer)");
        }
        return getter;
    }

    void FrameElement::remove() {
        if (auto a = dynamic_cast<ArticleElement *>(parent)) {
            auto article = a->get();
            article->remove_frame(getter);
            article->write_to_file();
            delete getter;
            getter = nullptr;
        } else {
            throw std::runtime_error("error while removing article element (orphan element)");
        }
    }

    void FrameElement::output(std::ostream &os, tcms::ExportVariant variant) {
        switch (variant) {
            case PLAIN:
                os << PlainFrameElement(*this);
                break;
            case MARKDOWN:
                os << MarkdownFrameElement(*this);
                break;
            case HTML:
                os << HTMLFrameElement(*this);
                break;
        }
    }

    MetadataElement::MetadataElement(tcms::Metadata &metadata, Context &ctx) : metadata(metadata), ctx(ctx) {}

    ElementType MetadataElement::get_type() const {
        return METADATA;
    }

    Element *MetadataElement::resolve(const std::string &path) {
        auto split = strings::split(path, '/', 2);
        if (split.empty() || split[0] == ".") {
            return this;
        } else if (split[0] == "..") {
            return parent->resolve(split.size() == 2 ? split[1] : ".");
        } else {
            auto idx = strings::parse_number<std::vector<std::shared_ptr<Tag>>::size_type>(split[0]);
            if (metadata.get_tags().size() <= idx) {
                return nullptr;
            } else {
                auto element = new TagElement(metadata.get_tags()[idx], ctx);
                element->parent = this;
                return element->resolve(split.size() == 2 ? split[1] : ".");
            }
        }
    }

    Metadata &MetadataElement::get() const {
        return metadata;
    }

    void MetadataElement::remove() {
        metadata.clear();
        if (auto a = dynamic_cast<ArticleElement *>(parent)) {
            a->get()->write_to_file();
        } else {
            throw std::runtime_error("error while clearing metadata element (orphan)");
        }
    }

    void MetadataElement::output(std::ostream &os, tcms::ExportVariant variant) {
        switch (variant) {
            case PLAIN:
                for (auto f: get().get_tags()) {
                    auto e = TagElement(f, ctx);
                    os << PlainTagElement(e) << '\n';
                }
                break;
            case MARKDOWN:
                for (auto f: get().get_tags()) {
                    auto e = TagElement(f, ctx);
                    os << MarkdownTagElement(e) << '\n';
                }
                break;
            case HTML:
                html_os_prefix(os, "Metadata");
                for (auto f: get().get_tags()) {
                    auto e = TagElement(f, ctx);
                    os << HTMLTagElement(e) << '\n';
                }
                html_os_suffix(os);
                break;
        }
    }

    bool MetadataElement::operator==(const tcms::Element *other) {
        if (auto m = dynamic_cast<const MetadataElement *>(other)) {
            return m->metadata == metadata;
        }
        return false;
    }

    TagElement::TagElement(tcms::Tag *tag, tcms::Context &ctx) : tag(tag), ctx(ctx) {}

    bool TagElement::operator==(const tcms::Element *other) {
        if (auto u = dynamic_cast<const TagElement *>(other)) {
            return tag == u->tag;
        }
        return false;
    }

    ElementType TagElement::get_type() const {
        return ElementType::TAG;
    }

    Element *TagElement::resolve(const std::string &path) {
        auto split = strings::split(path, '/', 2);
        if (split.empty() || split.size() == 1 && split[0] == ".") {
            return this;
        } else if (split.size() == 2 && split[0] == ".") {
            return resolve(split[1]);
        } else if (split[0] == "..") {
            return parent->resolve(split.size() == 2 ? split[1] : "");
        }
        return nullptr;
    }

    Tag *TagElement::get() const {
        if (tag == nullptr) {
            throw std::runtime_error("error while getting language tag (null pointer)");
        }
        return tag;
    }

    void TagElement::remove() {
        if (auto m = dynamic_cast<MetadataElement *>(parent)) {
            auto &metadata = m->get();
            metadata.remove_tag(tag);
            delete tag;
            tag = nullptr;
            if (auto a = dynamic_cast<ArticleElement *>(parent->parent)) {
                auto article = a->get();
                article->write_to_file();
            } else {
                throw std::runtime_error("error while removing language tag (orphaned metadata)");
            }
        } else {
            throw std::runtime_error("error while removing language tag (orphan)");
        }
    }

    void TagElement::output(std::ostream &os, tcms::ExportVariant variant) {
        switch (variant) {
            case PLAIN:
                os << PlainTagElement(*this);
                break;
            case MARKDOWN:
                os << MarkdownTagElement(*this);
                break;
            case HTML:
                os << HTMLTagElement(*this);
                break;
        }
    }
}