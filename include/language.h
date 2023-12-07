#ifndef TCMS_LANG_H
#define TCMS_LANG_H

#include <string>

class Language {
    char lang_code[3], locale_code[3];

    Language(const char language_code[3], const char locale_code[3]);

public:
    std::string get_language_code() const;

    std::string get_locale_code() const;

    bool operator==(const Language &other);

    static Language parse(const std::string &expr);
};

#endif