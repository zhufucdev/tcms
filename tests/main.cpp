#include "tcms.h"
#include "test.h"

void test_language() {
    auto lang = Language::parse("zh_CN");
    test::assert_eq("parse_language", "zh", lang.get_language_code());
    test::assert_eq("parse_country", "CN", lang.get_locale_code());
}

void test_contact() {
    tcms::Contact contact("John");
    contact.set_name(1, "Cena");
    test::assert_eq("get_full_name_contact", contact.get_full_name(), "John Cena");
    auto ba = contact.serialize();
    auto dsc = tcms::Contact::deserialize(ba);
    test::assert_eq("deserialize_contact", dsc->get_names(), contact.get_names());
}

int main() {
    test_language();
    test::complete();
}