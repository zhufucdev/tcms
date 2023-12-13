#include "metadata.h"
#include"language.h"
#include"contact.h"

using namespace tcms;

Tag::Tag(id_type id) : id(id) {}

LanguageTag::LanguageTag(id_type id, Language language) : lang(language), Tag(id) {}

AuthorTag::AuthorTag(id_type id,Contact* author):author(author),Tag(id){}

tcms::Metadata::Metadata(id_type id) {}

std::string tcms::Tag::to_string() const
{
	return std::string();
}

Language tcms::LanguageTag::get_language() const
{
	return lang;
}

ByteArray tcms::LanguageTag::serialize() const
{
	size_t len = sizeof(id_type)+5;
	char* buf = (char*)calloc(len, sizeof(char));
	bytes::write_number(buf, id);
	std::memcpy(buf + sizeof(id_type), lang.get_locale_code().c_str(), 2);
	std::memcpy(buf + sizeof(id_type) + 2, lang.get_language_code().c_str(), 2);
	return { buf, len };
}

std::string tcms::LanguageTag::to_string() const
{
	return std::string();
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

Contact* tcms::AuthorTag::get_author() const
{
	return author;
}

ByteArray tcms::AuthorTag::serialize() const
{
	ByteArray ba = author->serialize();
	size_t len = sizeof(id_type) + 5;
	char* buf = (char*)calloc(len, sizeof(char));
	bytes::write_number(buf, id);
	std::memcpy(buf + sizeof(id_type), ba.content,5);
	return { buf, len };
}

std::string tcms::AuthorTag::to_string() const
{
	return std::string();
}

AuthorTag tcms::AuthorTag::deserialize(ByteArray ba)
{
	id_type id = bytes::read_number<id_type>(ba.content);
	Contact* con = Contact::deserialize({ ba.content + sizeof(id_type),5 - sizeof(id_type) });
	return AuthorTag(id, con);
}

ByteArray tcms::Metadata::serialize() const
{
	ByteArray* serializedTags = new ByteArray[tags.size()];
	size_t totalTagSize = 0;
	for (size_t i = 0; i < tags.size(); ++i) {
		serializedTags[i] = tags[i]->serialize();
		totalTagSize += serializedTags[i].size;
	}
	size_t len = sizeof(id_type) + totalTagSize;
	char* buf = (char*)calloc(len, sizeof(char));
	bytes::write_number(buf, id);
	size_t currentPos = sizeof(id_type);
	for (size_t i = 0; i < tags.size(); ++i) {
		std::memcpy(buf + currentPos, serializedTags[i].content, serializedTags[i].size);
		currentPos += serializedTags[i].size;
		free(serializedTags[i].content);
	}
	delete[] serializedTags;
	return { buf, len };
}

Metadata tcms::Metadata::deserialize(ByteArray ba)
{
	return Metadata();
}