#include <fstream>
#include "test.h"
#include "fs.h"
#include "metadata.h"
#include "terminal.h"
#include "FrameGetter.h"
#include "ContactGetter.h"
#include "strhelper.h"

std::map<id_type, size_t> tcms::FrameGetter::rc{};
std::map<id_type, tcms::Frame *> tcms::FrameGetter::cache{};

std::map<id_type, size_t> tcms::ContactGetter::rc{};
std::map<id_type, tcms::Contact *> tcms::ContactGetter::cache{};


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

void test_fs_path_covrt() {
    test::assert_eq("path_to_string_1", "/var/log/mginx/latest.log",
                    fs::path_to_string(fs::Path{"/", "var", "log", "mginx", "latest.log"}));
    test::assert_eq("path_to_string_2", ".local/share", fs::path_to_string({".local", "share"}));
    test::assert_eq("string_to_path", fs::Path{"/", "a", "b", "cde"}, fs::string_to_path("/a/b/cde"));
}

void test_fs_list_files() {
    fs::create_directory("test_fs_list");
    for (int i = 0; i < 5; ++i) {
        std::ofstream ofs("test_fs_list/file" + std::to_string(i));
        ofs << "Greetings" << std::endl;
        ofs.close();
    }
    int count = 0;
    for (auto path: fs::list_files({"test_fs_list"})) {
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
    auto author = tcms::Contact("John");
    author.set_name(1, "Cena");

    metadata.add_tag(new tcms::LanguageTag(lang));
    metadata.add_tag(new tcms::AuthorTag(&author));
    auto ba = metadata.serialize();
    metadata = tcms::Metadata::deserialize(ba);
    test::assert_eq("metadata_lang_tag", lang.to_string(), metadata.get_tags()[0]->to_string());
    test::assert_eq("metadata_author_tag", author.get_full_name(), metadata.get_tags()[1]->to_string());
}

void test_terminal() {
    auto read = terminal::read_name({"lore\\", "ipsum", "hinc"}, 0);
    test::assert_eq("terminal_read_name_1", "lore ipsum", read.name);
    read = terminal::read_name({"touc\\", "lor"}, 0);
    test::assert_eq("terminal_read_name_2", "touc lor", read.name);
    test::assert_eq("terminal_read_para", "you are nerd", terminal::read_paragraph({"you", "are", "nerd"}, 0));
}

void test_strings_match() {
    auto src = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam";
    test::assert("strings_match_p",  strings::match(src, "dolor"));
    test::assert("strings_match_n", !strings::match(src, "lorem"));
    test::assert("strings_match_regex_p", strings::match(src, "[sadipcng]+", false, true));
    test::assert("strings_match_regex_n", !strings::match(src, "[A-C]+", false, true));
    test::assert("strings_ci_match_p", strings::match(src, "lOrEm", true));
    test::assert("strings_ci_match_n", !strings::match(src, "lOrM", true));
}

void test_strings_split() {
    test::assert_eq("strings_split_limit_2", strings::split("../touch/grass", "/", 2),
                    std::vector<std::string>{"..", "touch/grass"});
}

int main() {
    test::run_tests({test_language, test_contact,
                     test_fs_get_extension, test_fs_list_files,
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
                     test_fs_path_covrt,
#endif
                     test_metadata, test_terminal, test_strings_match,
                     test_strings_split});
}