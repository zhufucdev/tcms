#ifndef TCMS_METADATA_H
#define TCMS_METADATA_H

#include <string>
#include "language.h"
#include "ContactGetter.h"

namespace tcms {
    class Tag : public bytes::BinarySerializable {
    public:
        virtual std::string to_string() const = 0;
        virtual ~Tag() = default;
        static Tag *deserialize(ByteArray ba);
    };

    class LanguageTag : public Tag {
        Language lang;
    public:
        explicit LanguageTag(Language language);

        ~LanguageTag() override = default;

        Language get_language() const;

        ByteArray serialize() const override;

        std::string to_string() const override;

        static LanguageTag *deserialize(ByteArray ba);
    };

    class AuthorTag : public Tag {
        ContactGetter *author;
    public:
        explicit AuthorTag(id_type author_id);

        AuthorTag(ContactGetter *getter);

        ~AuthorTag() override = default;

        Contact *get_author() const;

        ByteArray serialize() const override;

        std::string to_string() const override;

        static AuthorTag *deserialize(ByteArray ba);
    };

    class Metadata : bytes::BinarySerializable {
        id_type id;
        std::vector<Tag *> tags;

        Metadata(id_type id, const std::vector<Tag *> &tags);

    public:
        Metadata();

        ~Metadata();

        ByteArray serialize() const override;

        std::vector<Tag *> get_tags() const;

        void add_tag(Tag *tag);

        static Metadata deserialize(ByteArray ba);
    };
}

#endif