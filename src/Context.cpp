#include "tcms.h"
#include <iostream>

namespace tcms {
    Context::Context() : articles(), cwe(nullptr) {
        fs::create_directory("content");
        fs::create_directory("frames");
        fs::create_directory("contacts");
        fs::create_directory("images");

        for (auto file: fs::list_files(fs::Path{"content"})) {
            if (fs::is_hidden(file)) {
                continue;
            }
            auto ba = fs::read_file(file);
            try {
                auto article = Article::deserialize(ba);
                articles.push_back(article);
            } catch (const std::exception &any) {
                std::cerr << "Error while reading " << fs::path_to_string(file) << ": "
                          << any.what() << std::endl;
            }
        }
    }

    Context::~Context() {
        for (auto a: articles) {
            delete a;
        }
    }

    void Context::alter_cwe(tcms::Element *new_cwe) {
        auto a = cwe;
        cwe = new_cwe;
        std::vector<Element *> removal;
        while (a != nullptr) {
            if (new_cwe != nullptr) {
                auto b = new_cwe;
                while (a != b && b != nullptr) {
                    b = b->parent;
                }
                if (b != nullptr) {
                    break;
                }
            }
            removal.push_back(a);
            a = a->parent;
        }
        for (auto const &r: removal) {
            delete r;
        }
    }

    Element *Context::get_current_working_element() const {
        return cwe;
    }
}
