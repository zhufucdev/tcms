#ifndef TCMS_H
#define TCMS_H

#include "Article.h"
#include "export.h"

namespace tcms {
    enum ElementType {
        ROOT,
        ARTICLE,
        FRAME,
        METADATA,
        CONTACT,
        TAG,
    };

    enum ExportVariant {
        PLAIN,
        MARKDOWN,
        HTML
    };

    class Element {
    public:
        Element *parent;

        explicit Element();

        virtual ~Element() = default;

        virtual bool operator==(const Element *other) = 0;

        virtual ElementType get_type() const = 0;

        virtual void remove() = 0;

        virtual void output(std::ostream &os, ExportVariant variant) = 0;

        virtual Element *resolve(const std::string &path) = 0;
    };

    class Context {
        Element *cwe;
    public:
        std::vector<Article *> articles;

        Context();

        ~Context();

        void alter_cwe(Element *cwe);

        bool is_running() const;

        Element *get_current_working_element() const;
    };

    class CliClient {
        Context &ctx;

    public:
        explicit CliClient(Context &ctx);

        void event_loop();

        void interrupt(int signal);
    };

    class RootElement : public Element {
        Context &ctx;
    public:
        explicit RootElement(Context &ctx);

        bool operator==(const Element *other) override;

        ElementType get_type() const override;

        void remove() override;

        Element *resolve(const std::string &path) override;

        void output(std::ostream &os, tcms::ExportVariant variant) override;
    };

    class ArticleElement : public Element {
        Article *article;
        Context &ctx;
    public:
        ArticleElement(Article *article, Context &ctx);

        ArticleElement(const std::string &name, Context &ctx);

        void remove() override;

        bool operator==(const Element *other) override;

        Article *get() const;

        ElementType get_type() const override;

        Element *resolve(const std::string &path) override;

        void output(std::ostream &os, tcms::ExportVariant variant) override;
    };

#define ProduceElementType(Name, Holder, Var) \
    class Name : public Element { \
        Holder Var; \
        Context &ctx; \
    public: \
        Name(Holder Var, Context &ctx); \
        void remove() override; \
        bool operator==(const Element *other) override; \
        Holder get() const; \
        ElementType get_type() const override; \
        Element *resolve(const std::string &path) override; \
        void output(std::ostream &os, tcms::ExportVariant variant) override; \
    }

    ProduceElementType(FrameElement, FrameGetter*, getter);

    ProduceElementType(MetadataElement, Metadata &, metadata);

    ProduceElementType(TagElement, Tag*, tag);

    namespace behavior {
        class Behavior {
        protected:
            const Context &ctx;
        public:
            explicit Behavior(const Context &ctx);
        };

        class ListArticle : public Behavior {
            const Article *article;
            const bool detailed, separator, dot_name;

        public:
            ListArticle(const Context &ctx, const Article *article, bool detailed, bool separator = true,
                        bool dot_name = false);

            friend std::ostream &operator<<(std::ostream &os, const ListArticle &lr);
        };

        class ListInRoot : public Behavior {
            const bool detailed, all;

        public:
            explicit ListInRoot(const Context &ctx, bool detailed, bool all);

            friend std::ostream &operator<<(std::ostream &os, const ListInRoot &lr);
        };

        class ListInArticle : public Behavior {
            const Article *article;
            const bool detailed, all;
            const unsigned char type;

        public:
            explicit ListInArticle(const Context &ctx, const Article *article, bool detailed, bool all,
                                   unsigned char type_filter);

            friend std::ostream &operator<<(std::ostream &os, const ListInArticle &la);
        };

        class ListMetadata : public Behavior {
            const Metadata &metadata;
            const bool detailed, dot_name;

        public:
            ListMetadata(const Context &ctx, const Metadata &metadata, bool detailed, bool dot_name);

            friend std::ostream &operator<<(std::ostream &os, const ListMetadata &lm);
        };

        class ListInMetadata : public Behavior {
            const Metadata &metadata;
            const bool detailed, all;
            const unsigned char type;

        public:
            ListInMetadata(const Context &ctx, const Metadata &metadata, bool detailed, bool all,
                           unsigned char type_filer);

            friend std::ostream &operator<<(std::ostream &os, const ListInMetadata &lm);
        };

        class ListInElement : public Behavior {
            const Element *element;
            const bool detailed, all;
            const unsigned char type;

        public:
            explicit ListInElement(const Context &ctx, const Element *ele, bool detailed, bool all,
                                   unsigned char type_filter);

            friend std::ostream &operator<<(std::ostream &os, const ListInElement &le);
        };

    }

    Ostreamable(FrameElement &, MarkdownFrameElement);

    Ostreamable(FrameElement &, PlainFrameElement);

    Ostreamable(FrameElement &, HTMLFrameElement);

    Ostreamable(TagElement &, PlainTagElement);

    Ostreamable(TagElement &, HTMLTagElement);

    Ostreamable(TagElement &, MarkdownTagElement);
}

#endif