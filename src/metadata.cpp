#include "metadata.h"
#include"language.h"
#include"contact.h"

using namespace tcms;

Tag::Tag(id_type id) : id(id) {}

LanguageTag::LanguageTag(id_type id, Language language) : lang(language), Tag(id) {}

AuthorTag::AuthorTag(id_type id, Contact *author) : author(author), Tag(id) {}

std::string tcms::LanguageTag::to_string() const {
    return lang.to_string();
}

Language tcms::LanguageTag::get_language() const {
    return lang;
}

Contact *tcms::AuthorTag::get_author() const {
    return author;
}

ByteArray AuthorTag::serialize() const {
    return ByteArray();
}

std::string AuthorTag::to_string() const {
    return std::string();
}

AuthorTag AuthorTag::deserialize(ByteArray ba) {
    return AuthorTag(0, nullptr);
}

ByteArray tcms::LanguageTag::serialize() const {
    size_t len = sizeof(id_type) + 5;
    char *buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    std::memcpy(buf + sizeof(id_type), lang.get_locale_code().c_str(), 2);
    std::memcpy(buf + sizeof(id_type) + 2, lang.get_language_code().c_str(), 2);
    return {buf, len};
}

LanguageTag LanguageTag::deserialize(ByteArray ba)
{
	id_type id = bytes::read_number<id_type>(ba.content);
	char locale_code[3] = { 0 };
	char language_code[3] = { 0 };
	std::memcpy(locale_code, ba.content + sizeof(id_type), 2);
	std::memcpy(language_code, ba.content + sizeof(id_type) + 2, 2);
	Language language{ locale_code, language_code };
	return LanguageTag{id, language};
}

Metadata::Metadata(id_type id) : id(id) {}

Metadata::~Metadata() {
    for (auto t : tags) {
        delete t;
    }
}

ByteArray tcms::Metadata::serialize() const
{
	auto* serializedTags = new ByteArray[tags.size()];
	size_t totalTagSize = 0;
	for (size_t i = 0; i < tags.size(); ++i) {
		serializedTags[i] = tags[i]->serialize();
		totalTagSize += serializedTags[i].len;
	}
	size_t len = sizeof(id_type) + totalTagSize;
	char* buf = (char*)calloc(len, sizeof(char));
	bytes::write_number(buf, id);
	size_t currentPos = sizeof(id_type);
	for (size_t i = 0; i < tags.size(); ++i) {
		std::memcpy(buf + currentPos, serializedTags[i].content, serializedTags[i].len);
		currentPos += serializedTags[i].len;
		free(serializedTags[i].content);
	}
	delete[] serializedTags;
	return { buf, len };
}

Metadata tcms::Metadata::deserialize(ByteArray ba)
{
	id_type id = bytes::read_number<id_type>(ba.content);
	std::vector<Tag*> tags;
	size_t currentPos = sizeof(id_type);
	while (currentPos < ba.len) {
		AuthorTag* newTag = new AuthorTag(AuthorTag::deserialize({ ba.content + currentPos, ba.len - currentPos }));
		tags.push_back(newTag);
		currentPos += newTag->serialize().len;
	}
	auto m = Metadata(id);
	m.tags = tags;
	return m;
}