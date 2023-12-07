#ifndef TCMS_METADATA_H
#define TCMS_METADATA_H

#include <string>
#include "language.h"
#include "contact.h"

namespace tcms {
    class Tag : public bytes::BinarySerializable {
        id_type id;
    public:
        explicit Tag(int id);
        virtual std::string to_string() const = 0;
    };

    class LanguageTag : public Tag {
        Language lang;
    public:
        LanguageTag(Language language);
        Language get_language() const;
        ByteArray serialize() const override;
        std::string to_string() const override;
        static LanguageTag deserialize(ByteArray ba);
    };

    class AuthorTag : public Tag {
        Contact *author;
    public:
        AuthorTag(Contact *author);
        Contact *get_author() const;
        ByteArray serialize() const override;
        std::string to_string() const override;
        static AuthorTag deserialize(ByteArray ba);
    };

    class Metadata : bytes::BinarySerializable {
        id_type id;
        std::vector<Tag *> tags;
    public:
        Metadata(int id);
        ~Metadata();
        ByteArray serialize() const override;
        static Metadata deserialize(ByteArray ba);
    };
}

#endif