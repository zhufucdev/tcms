#include "tcms.h"
#include "test.h"

void test_language() {
    auto lang = Language::parse("zh_CN");
    test::assert_eq("parse_language", "zh", lang.get_language_code());
    test::assert_eq("parse_country", "CN", lang.get_locale_code());
}

int main() {
    test_language();
    test::complete();
}