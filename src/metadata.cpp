#include "metadata.h"
#include"language.h"
#include"contact.h"

using namespace tcms;

Tag::Tag(id_type id) : id(id) {}

LanguageTag::LanguageTag(id_type id, Language language) : lang(language), Tag(id) {}

AuthorTag::AuthorTag(id_type id,Contact* author):author(author),Tag(id){}

std::string tcms::LanguageTag::to_string() const
{
	return std::string();
}

Language tcms::LanguageTag::get_language() const
{
	return lang;
}

Contact* tcms::AuthorTag::get_author() const
{

	return author;
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