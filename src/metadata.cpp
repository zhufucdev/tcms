#include "metadata.h"
#include"language.h"
#include"Contact.h"

using namespace tcms;

enum TagType {
    Lang = 0,
    Author,
    Title
};

Tag *Tag::deserialize(ByteArray ba) {
    switch (ba.content[0]) {
        case TagType::Lang:
            return LanguageTag::deserialize(ba);
        case TagType::Author:
            return AuthorTag::deserialize(ba);
        default:
            throw std::runtime_error("Unknown type (deserializing Tag)");
    }
}

LanguageTag::LanguageTag(Language language) : lang(language), Tag() {}

ByteArray tcms::LanguageTag::serialize() const {
    auto str = lang.to_string();
    auto len = str.length() + 2;
    auto buf = (char *) calloc(len, sizeof(char));
    buf[0] = TagType::Lang;
    std::memcpy(buf + 1, str.c_str(), str.length());
    return {buf, len};
}

LanguageTag *LanguageTag::deserialize(ByteArray ba) {
    if (ba.content[0] != TagType::Lang) {
        throw std::runtime_error("Unexpected header (deserializing LanguageTag)");
    }
    auto str = std::string(ba.content + 1);
    return new LanguageTag{Language::parse(str)};
}


std::string tcms::LanguageTag::to_string() const {
    return lang.to_string();
}

Language tcms::LanguageTag::get_language() const {
    return lang;
}

AuthorTag::AuthorTag(id_type author_id) : author(new ContactGetter(author_id)), Tag() {}

AuthorTag::AuthorTag(tcms::ContactGetter *getter) : author(getter) {}

Contact *tcms::AuthorTag::get_author() const {
    return author->get();
}

std::string AuthorTag::to_string() const {
    return author->get()->get_full_name();
}

ByteArray AuthorTag::serialize() const {
    auto cid = author->get_id();
    auto buf = (char *) malloc(sizeof(size_t) + 1);
    buf[0] = TagType::Author;
    bytes::write_number(buf + 1, cid);
    return {buf, sizeof(size_t)};
}

AuthorTag *AuthorTag::deserialize(ByteArray ba) {
    if (ba.content[0] != TagType::Author) {
        throw std::runtime_error("Unexpected header (deserializing AuthorTag)");
    }
    auto cid = bytes::read_number<id_type>(ba.content + 1);
    return new AuthorTag(cid);
}

Metadata::Metadata(id_type id, const std::vector<Tag *> &tags) : id(id), tags(tags) {}

Metadata::Metadata() : id(increment::get_next_id()), tags() {}

Metadata::~Metadata() {
    for (auto tag : tags) {
        delete tag;
    }
}

std::vector<Tag *> Metadata::get_tags() const {
    return tags;
}

ByteArray tcms::Metadata::serialize() const {
    auto *serializedTags = new ByteArray[tags.size()];
    size_t totalTagSize = 0;
    for (size_t i = 0; i < tags.size(); ++i) {
        serializedTags[i] = tags[i]->serialize();
        totalTagSize += serializedTags[i].len;
    }
    size_t len = sizeof(id_type) + totalTagSize + sizeof(size_t) * tags.size();
    char *buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    size_t currentPos = sizeof(id_type);
    for (size_t i = 0; i < tags.size(); ++i) {
        bytes::write_number(buf + currentPos, serializedTags[i].len);
        std::memcpy(buf + currentPos + sizeof(size_t), serializedTags[i].content, serializedTags[i].len);
        currentPos += serializedTags[i].len + sizeof(size_t);
        delete serializedTags[i].content;
    }
    delete[] serializedTags;
    return {buf, len};
}

Metadata tcms::Metadata::deserialize(ByteArray ba) {
    auto id = bytes::read_number<id_type>(ba.content);
    std::vector<Tag *> tags;
    size_t current_pos = sizeof(id_type);
    while (current_pos < ba.len) {
        auto curr_size = bytes::read_number<size_t>(ba.content + current_pos);
        tags.push_back(Tag::deserialize(ba + current_pos + sizeof(size_t)));
        current_pos += curr_size + sizeof(size_t);
    }
    return {id, tags};
}

void Metadata::add_tag(Tag *tag) {
    tags.push_back(tag);
}
