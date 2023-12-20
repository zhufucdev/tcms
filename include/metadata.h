#ifndef TCMS_METADATA_H
#define TCMS_METADATA_H

#include <string>
#include "language.h"
#include "ContactGetter.h"

namespace tcms {
    class Tag : public bytes::BinarySerializable {
    public:
        virtual std::string to_string() const = 0;
        static Tag *deserialize(ByteArray ba);
    };

    class LanguageTag : public Tag {
        Language lang;
    public:
        explicit LanguageTag(Language language);

        Language get_language() const;

        ByteArray serialize() const override;

        std::string to_string() const override;

        static LanguageTag *deserialize(ByteArray ba);
    };

    class AuthorTag : public Tag {
        ContactGetter author;
    public:
        explicit AuthorTag(id_type author_id);

        Contact *get_author() const;

        ByteArray serialize() const override;

        std::string to_string() const override;

        static AuthorTag *deserialize(ByteArray ba);
    };

    class Metadata : bytes::BinarySerializable {
        id_type id;
        std::vector<Tag *> tags;
    public:
        explicit Metadata(id_type id);

        ~Metadata();

        ByteArray serialize() const override;

        std::vector<Tag *> get_tags() const;

        static Metadata deserialize(ByteArray ba);
    };
}

#endif