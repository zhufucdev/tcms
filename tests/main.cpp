#include <fstream>
#include "TCMS.h"
#include "test.h"
#include "fs.h"
#include "metadata.h"

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

void test_fs_get_extension() {
    test::assert_eq("get_extension", ".png", fs::get_extension(fs::Path{"I_like_bingchilling.png"}));
    test::assert_eq("get_empty_extension", ".", fs::get_extension(fs::Path{"IDK"}));
}

void test_fs_list_files() {
    fs::create_directory("test_fs_list");
    for (int i = 0; i < 5; ++i) {
        std::ofstream ofs("test_fs_list/file" + std::to_string(i));
        ofs << "Greetings" << std::endl;
        ofs.close();
    }
    int count = 0;
    for (auto path : fs::list_files({"test_fs_list"})) {
        if (path[1][0] == '.') {
            continue;
        }
        test::assert_eq("fs_list_get_name_dir", "test_fs_list", path[0]);
        test::assert_eq("fs_list_get_name_prefix", "file", path[1].substr(0, path[1].length() - 1));
        auto n = path[1][path[1].length() - 1] - '0';
        test::assert("fs_list_get_name_suffix", n >= 0 && n <= 5);
        count++;
    }
    test::assert_eq("fs_list_count", 5, count);
}

void test_metadata() {
    tcms::Metadata metadata;
    tcms::LanguageTag lang(Language::parse("zh_CN"));
    tcms::ConstantContactGetter at(new tcms::Contact("John"));
    at.get()->set_name(1, "Cena");

    metadata.add_tag(new tcms::LanguageTag(lang));
    metadata.add_tag(new tcms::AuthorTag(&at));
    auto ba = metadata.serialize();
    metadata = tcms::Metadata::deserialize(ba);
    test::assert_eq("metadata_lang_tag", lang.to_string(), metadata.get_tags()[0]->to_string());
    test::assert_eq("metadata_author_tag", at.get()->get_full_name(), metadata.get_tags()[1]->to_string());
}

int main() {
    test::run_tests({test_language, test_contact,
                     test_fs_get_extension, test_fs_list_files,
                     test_metadata});
}