#include <string>
#include "language.h"
#include "contact.h"

namespace tcms {
    class Tag {
        int id;
    public:
        explicit Tag(int id);
        virtual std::string to_string() const = 0;
        virtual ByteArray serialize() const = 0;
    };

    class LanguageTag : public Tag {
        Language lang;
    public:
        LanguageTag(Language language);
        Language get_language() const;
        static LanguageTag deserialize(ByteArray ba);
    };

    class AuthorTag : public Tag {
        Contact *author;
    public:
        AuthorTag(Contact *author);
        Contact *get_author() const;
        static AuthorTag deserialize(ByteArray ba);
    };

    class Metadata {
        int id;
        std::vector<Tag *> tags;
    public:
        Metadata(int id);
        ~Metadata();
        ByteArray serialize();
        static Metadata deserialize(ByteArray ba);
    };
}