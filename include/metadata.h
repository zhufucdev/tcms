#ifndef TCMS_METADATA_H
#define TCMS_METADATA_H

#include <string>
#include "language.h"
#include "ContactGetter.h"
#include <memory>

namespace tcms {
    enum TagType {
        LANG = 0b00000001,
        AUTHOR = 0b00000010,
        TITLE = 0b00000100
    };

    class Tag : public bytes::BinarySerializable {
    public:
        virtual std::string to_string() const = 0;

        virtual ~Tag() = default;

        virtual TagType get_type() const = 0;

        static Tag *deserialize(ByteArray ba);
    };

    class LanguageTag : public Tag {
        Language lang;
    public:
        explicit LanguageTag(Language language);

        ~LanguageTag() override = default;

        Language get_language() const;

        TagType get_type() const override;

        ByteArray serialize() const override;

        std::string to_string() const override;

        bool operator==(const LanguageTag &other) const;

        static LanguageTag *deserialize(ByteArray ba);
    };

    class AuthorTag : public Tag {
        ContactGetter *author;
    public:
        explicit AuthorTag(id_type author_id);

        explicit AuthorTag(Contact *contact);

        explicit AuthorTag(ContactGetter *getter);

        ~AuthorTag() override = default;

        Contact *get_author() const;

        TagType get_type() const override;

        ByteArray serialize() const override;

        std::string to_string() const override;

        bool operator==(const AuthorTag &other);

        static AuthorTag *deserialize(ByteArray ba);
    };

    class TitleTag : public Tag {
        std::string title;
    public:
        explicit TitleTag(const std::string &title);

        TagType get_type() const override;

        ByteArray serialize() const override;

        std::string to_string() const override;

        static TitleTag *deserialize(ByteArray ba);
    };

    class Metadata : bytes::BinarySerializable {
        std::vector<Tag *> tags;

        explicit Metadata(const std::vector<Tag *> &tags);

    public:
        Metadata();

        ~Metadata();

        ByteArray serialize() const override;

        std::vector<Tag *> get_tags() const;

        void add_tag(Tag *tag);

        void remove_tag(Tag *tag);

        void clear();

        bool operator==(const Metadata &other) const;

        static Metadata deserialize(ByteArray ba);
    };
}

#endif