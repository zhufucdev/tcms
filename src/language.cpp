#include "language.h"
#include <regex>

Language::Language(const char *language_code, const char *locale_code) {
    std::strcpy(lang_code, language_code);
    std::strcpy(this->locale_code, locale_code);
}

std::string Language::get_locale_code() const {
    return {locale_code};
}

std::string Language::get_language_code() const {
    return {lang_code};
}

bool Language::operator==(const Language &other) {
    return lang_code == other.lang_code && locale_code == other.locale_code;
}

std::string Language::to_string() const {
    char *buf = (char *) calloc(sizeof lang_code + sizeof locale_code - 2, sizeof(char));
    std::memcpy(buf, lang_code, sizeof lang_code - 1);
    buf[sizeof lang_code - 1] = '_';
    std::memcpy(buf + sizeof lang_code, locale_code, sizeof locale_code);
    return buf;
}

const static char language_codes[][3] = {"aa", "ab", "af", "ak", "sq", "am", "ar", "an", "hy", "as", "av", "ae",
                                         "ay", "az", "ba", "bm", "eu", "be", "bn", "bh", "bi", "bo", "bs", "br",
                                         "bg", "my", "ca", "cs", "ch", "ce", "zh", "cu", "cv", "kw", "co", "cr",
                                         "cy", "cs", "da", "de", "dv", "nl", "dz", "el", "en", "eo", "et", "eu",
                                         "ee", "fo", "fa", "fj", "fi", "fr", "fr", "fy", "ff", "ka", "de", "gd",
                                         "ga", "gl", "gv", "el", "gn", "gu", "ht", "ha", "he", "hz", "hi", "ho",
                                         "hr", "hu", "hy", "ig", "is", "io", "ii", "iu", "ie", "ia", "id", "ik",
                                         "is", "it", "jv", "ja", "kl", "kn", "ks", "ka", "kr", "kk", "km", "ki",
                                         "rw", "ky", "kv", "kg", "ko", "kj", "ku", "lo", "la", "lv", "li", "ln",
                                         "lt", "lb", "lu", "lg", "mk", "mh", "ml", "mi", "mr", "ms", "mk", "mg",
                                         "mt", "mn", "mi", "ms", "my", "na", "nv", "nr", "nd", "ng", "ne", "nl",
                                         "nn", "nb", "no", "ny", "oc", "oj", "or", "om", "os", "pa", "fa", "pi",
                                         "pl", "pt", "ps", "qu", "rm", "ro", "ro", "rn", "ru", "sg", "sa", "si",
                                         "sk", "sk", "sl", "se", "sm", "sn", "sd", "so", "st", "es", "sq", "sc",
                                         "sr", "ss", "su", "sw", "sv", "ty", "ta", "tt", "te", "tg", "tl", "th",
                                         "bo", "ti", "to", "tn", "ts", "tk", "tr", "tw", "ug", "uk", "ur", "uz",
                                         "ve", "vi", "vo", "cy", "wa", "wo", "xh", "yi", "yo", "za", "zh", "zu"};
const static char locale_codes[][3] = {"AE", "AF", "AG", "AI", "AL", "AM", "AO", "AQ", "AR", "AS", "AT", "AU", "AW",
                                       "AX", "AZ", "BA", "BB", "BD", "BE", "BF", "BG", "BH", "BI", "BJ", "BL", "BM",
                                       "BN", "BO", "BQ", "BR", "BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC", "CD",
                                       "CF", "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", "CR", "CU", "CV", "CW",
                                       "CX", "CY", "CZ", "DE", "DJ", "DK", "DM", "DO", "DZ", "EC", "EE", "EG", "EH",
                                       "ER", "ES", "ET", "FI", "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE",
                                       "GF", "GG", "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT", "GU",
                                       "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "ID", "IE", "IL", "IM", "IN",
                                       "IO", "IQ", "IR", "IS", "IT", "JE", "JM", "JO", "JP", "KE", "KG", "KH", "KI",
                                       "KM", "KN", "KP", "KR", "KW", "KY", "KZ", "LA", "LB", "LC", "LI", "LK", "LR",
                                       "LS", "LT", "LU", "LV", "LY", "MA", "MC", "MD", "ME", "MF", "MG", "MH", "MK",
                                       "ML", "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT", "MU", "MV", "MW", "MX",
                                       "MY", "MZ", "NA", "NC", "NE", "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU",
                                       "NZ", "OM", "PA", "PE", "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS",
                                       "PT", "PW", "PY", "QA", "RE", "RO", "RS", "RU", "RW", "SA", "SB", "SC", "SD",
                                       "SE", "SG", "SH", "SI", "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "SS", "ST",
                                       "SV", "SX", "SY", "SZ", "TC", "TD", "TF", "TG", "TH", "TJ", "TK", "TL", "TM",
                                       "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG", "UM", "US", "UY", "UZ",
                                       "VA", "VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YT", "ZA",
                                       "ZM"};

Language Language::parse(const std::string &expr) {
    std::regex match_locale(R"(([a-z]{2})[_.]([A-Z]{2}))");
    std::smatch smatch;
    std::string locale, lang;
    if (std::regex_search(expr, smatch, match_locale)) {
        if (smatch.size() < 3) {
            throw std::runtime_error("illegal locale expression");
        }
        locale = smatch[2];
        lang = smatch[1];
    } else {
        throw std::runtime_error("illegal locale expression");
    }

    bool found = false;
    for (auto locale_code: locale_codes) {
        if (locale_code == locale) {
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error("unknown locale (" + locale + ")");
    }
    found = false;
    for (auto language_code: language_codes) {
        if (language_code == lang) {
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error("unknown language (" + lang + ")");
    }
    return {lang.c_str(), locale.c_str()};
}

