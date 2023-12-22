#include "metadata.h"
#include "language.h"
#include "Contact.h"
#include <stdexcept>

using namespace tcms;

Tag *Tag::deserialize(ByteArray ba) {
    switch (ba.content[0]) {
        case TagType::LANG:
            return LanguageTag::deserialize(ba);
        case TagType::AUTHOR:
            return AuthorTag::deserialize(ba);
        case TagType::TITLE:
            return TitleTag::deserialize(ba);
        default:
            throw std::runtime_error("unknown type (deserializing Tag)");
    }
}

LanguageTag::LanguageTag(Language language) : lang(language), Tag() {}

TagType LanguageTag::get_type() const {
    return TagType::LANG;
}

ByteArray tcms::LanguageTag::serialize() const {
    auto str = lang.to_string();
    auto len = str.length() + 2;
    auto buf = (char *) calloc(len, sizeof(char));
    buf[0] = TagType::LANG;
    std::memcpy(buf + 1, str.c_str(), str.length());
    return {buf, len};
}

LanguageTag *LanguageTag::deserialize(ByteArray ba) {
    if (ba.content[0] != TagType::LANG) {
        throw std::runtime_error("unexpected header (deserializing LanguageTag)");
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

bool LanguageTag::operator==(const tcms::LanguageTag &other) const {
    return static_cast<Language>(other.lang) == lang;
}

AuthorTag::AuthorTag(id_type author_id) : author(new ContactGetter(author_id)), Tag() {}

AuthorTag::AuthorTag(Contact *contact) : author(new MemoryContactGetter(contact)) {}

AuthorTag::AuthorTag(tcms::ContactGetter *getter) : author(getter) {}

ContactGetter *AuthorTag::get_author() const {
    return author;
}

TagType AuthorTag::get_type() const {
    return TagType::AUTHOR;
}

std::string AuthorTag::to_string() const {
    return author->get()->get_full_name();
}

ByteArray AuthorTag::serialize() const {
    auto cid = author->get_id();
    auto buf = (char *) malloc(sizeof(size_t) + 1);
    buf[0] = TagType::AUTHOR;
    bytes::write_number(buf + 1, cid);
    return {buf, sizeof(size_t)};
}

bool AuthorTag::operator==(const tcms::AuthorTag &other) {
    return author == other.author;
}

AuthorTag *AuthorTag::deserialize(ByteArray ba) {
    if (ba.content[0] != TagType::AUTHOR) {
        throw std::runtime_error("unexpected header (deserializing AuthorTag)");
    }
    auto cid = bytes::read_number<id_type>(ba.content + 1);
    return new AuthorTag(cid);
}

TitleTag::TitleTag(const std::string &title) : title(title) {}

TagType TitleTag::get_type() const {
    return TagType::TITLE;
}

std::string TitleTag::to_string() const {
    return title;
}

ByteArray TitleTag::serialize() const {
    auto buf = (char *) calloc(title.length() + 2, sizeof(char));
    buf[0] = TagType::TITLE;
    std::memcpy(buf + 1, title.c_str(), title.length());
    return {buf, title.length() + 2};
}

TitleTag *TitleTag::deserialize(ByteArray ba) {
    if (ba.content[0] != TagType::TITLE) {
        throw std::runtime_error("unexpected header (deserializing TitleTag)");
    }
    return new TitleTag({ba.content + 1});
}

Metadata::Metadata(const std::vector<Tag *> &tags) : tags(tags) {}

Metadata::Metadata() : tags() {}

Metadata::~Metadata() = default;

std::vector<Tag *> Metadata::get_tags() const {
    auto r = std::vector<Tag *>(tags.size());
    for (int i = 0; i < tags.size(); ++i) {
        r[i] = tags[i];
    }
    return r;
}

ByteArray tcms::Metadata::serialize() const {
    auto *serializedTags = new ByteArray[tags.size()];
    size_t totalTagSize = 0;
    for (size_t i = 0; i < tags.size(); ++i) {
        serializedTags[i] = (*tags[i]).serialize();
        totalTagSize += serializedTags[i].len;
    }
    size_t len = totalTagSize + sizeof(size_t) * tags.size();
    char *buf = (char *) calloc(len, sizeof(char));
    size_t currentPos = 0;
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
    std::vector<Tag *> tags;
    size_t current_pos = 0;
    while (current_pos < ba.len) {
        auto curr_size = bytes::read_number<size_t>(ba.content + current_pos);
        auto curr_ba = ba + current_pos + sizeof(size_t);
        if (curr_ba.len <= 0) {
            break;
        }
        auto sp = Tag::deserialize(curr_ba);
        tags.push_back(sp);
        current_pos += curr_size + sizeof(size_t);
    }
    return Metadata(tags);
}

void Metadata::add_tag(Tag *tag) {
    tags.push_back(tag);
}

void Metadata::remove_tag(tcms::Tag *tag) {
    tags.erase(std::find(tags.begin(), tags.end(), tag));
}

void Metadata::clear() {
    for (auto t: tags) {
        delete t;
    }
    tags.clear();
}

bool Metadata::operator==(const tcms::Metadata &other) const {
    return other.tags == tags;
}