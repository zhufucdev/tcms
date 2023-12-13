#include "tcms.h"
#include "test.h"
#include "fs.h"

void test_language() {
    auto lang = Language::parse("zh_CN");
    test::assert_eq("parse_language", "zh", lang.get_language_code());
    test::assert_eq("parse_country", "CN", lang.get_locale_code());
    test::assert_eq("test_to_string", "zh_CN", lang.to_string());
}

void test_contact() {
    tcms::Contact contact("John");
    contact.set_name(1, "Cena");
    test::assert_eq("get_full_name_contact", "John Cena", contact.get_full_name());
    auto ba = contact.serialize();
    auto dsc = tcms::Contact::deserialize(ba);
    test::assert_eq("deserialize_contact", contact.get_names(), dsc->get_names());
}

void test_fs() {
    test::assert_eq("get_extension", ".png", fs::get_extension(Path("I_like_bingchilling.png")));
    test::assert_eq("get_empty_extension", ".", fs::get_extension(Path("IDK")));
}

int main() {
    test::run_tests({test_language, test_contact, test_fs});
    test::complete();
}