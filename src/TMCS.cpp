#include "TCMS.h"
#include "fs.h"
#include "strings.h"
#include "terminal.h"
#include <iostream>

using namespace std;

tcms::TCMS::TCMS() : running(false), articles() {
    fs::create_directory("content");
    fs::create_directory("frames");
    fs::create_directory("contacts");

    for (auto file: fs::list_files(fs::Path{"content"})) {
        if (fs::is_hidden(file)) {
            continue;
        }
        auto ba = fs::read_file(file);
        try {
            auto article = Article::deserialize(ba);
            articles.push_back(article);
        } catch (const std::exception &any) {
            cerr << "Error while reading " << fs::path_to_string(file) << ": " << any.what() << endl;
        }
    }
}

tcms::TCMS::~TCMS() {
    for (auto a: articles) {
        delete a;
    }
}

enum CommandResult {
    SUCCESS = 0,
    FAILURE,
    EMPTY,
    NO_MATCH
};

template<typename Handler>
CommandResult handle_command(const vector<string> &args, const Handler &last_handler) {
    if (args[0] == get<0>(last_handler)) {
        return get<2>(last_handler)(args);
    } else {
        return CommandResult::NO_MATCH;
    }
}

template<typename Handler, typename ...Handlers>
CommandResult handle_command(const vector<string> args, const Handler &first_handler, const Handlers &...handlers) {
    if (args.empty()) {
        return CommandResult::EMPTY;
    } else if (get<0>(first_handler) == args[0]) {
        return get<2>(first_handler)(args);
    } else {
        return handle_command(args, handlers...);
    }
}

template<size_t I = 0, typename... Ts>
typename enable_if<I == sizeof...(Ts) - 1, void>::type
print_tuple_except_last(const tuple<Ts...> &) {}

template<size_t I = 0, typename... Ts>
typename enable_if<I < sizeof...(Ts) - 1, void>::type
print_tuple_except_last(const tuple<Ts...> &tuple) {
    cout << '[' << get<I>(tuple) << "] ";
    print_tuple_except_last<I + 1>(tuple);
}

template<typename ...Ts>
void print_usage(const tuple<Ts...> &manual) {
    print_tuple_except_last(manual);
    cout << "- " << get<sizeof...(Ts) - 1>(manual) << "\n";
}

template<typename Handler>
void print_help(Handler last_handler) {
    cout << "  " << get<0>(last_handler) << ' ';
    auto manual = get<1>(last_handler);
    print_usage(manual);
}

template<typename Handler, typename ...Handlers>
void print_help(const Handler &first_handler, const Handlers &...handlers) {
    print_help(first_handler);
    print_help(handlers...);
}

template<typename Handler, typename ...Handlers>
CommandResult handle_command(const string &input, const Handler &first_handler, const Handlers &...handlers) {
    auto args = strings::split(input, ' ');
    if (args.size() == 1 && args[0] == "?") {
        cout << "\nHelp:\n\n";
        print_help(first_handler, handlers...);
        cout << endl;
        return CommandResult::SUCCESS;
    } else {
        return handle_command(args, first_handler, handlers...);
    }
}

void print_dialog(const string &title, const string &body, int padding = 2) {
    static int min_line_width = 36, default_decorator_width = 4;
    static char hor_decorator = '=', vert_decorator = '|';
    auto decorator_len =
            title.length() + default_decorator_width * 2 + 2 < min_line_width ? min_line_width - title.length() - 2
                                                                              : default_decorator_width;
    for (int i = 0; i < decorator_len; ++i) {
        cout << hor_decorator;
    }
    cout << ' ' << title << ' ';
    for (int i = 0; i < decorator_len; ++i) {
        cout << hor_decorator;
    }
    cout << '\n';
    auto title_len = decorator_len * 2 + title.length() + 2;
    auto line_width = title_len - padding * 2;
    auto lines = body.length() / line_width + 1;
    size_t curr_ptr = 0;
    for (size_t i = 0; i < lines; ++i) {
        cout << vert_decorator;
        auto next_ptr = body.length() - curr_ptr > line_width ? curr_ptr + line_width : body.length();
        for (int j = 0; j < padding + (line_width - next_ptr + curr_ptr) / 2; ++j) {
            cout << ' ';
        }
        cout << body.substr(curr_ptr, next_ptr);
        for (int j = 0; j < padding + (line_width - next_ptr + curr_ptr) / 2; ++j) {
            cout << ' ';
        }
        cout << vert_decorator << '\n';
        curr_ptr += line_width;
    }
    for (size_t i = 0; i < title_len; ++i) {
        cout << hor_decorator;
    }
    cout << endl;
}

void tcms::TCMS::event_loop() {
    running = true;
    print_dialog("TCMS - The Content Management System", "Welcome to TCMS. Type ? for help");
    while (running) {
        if (cin.eof()) {
            break;
        }
        cout << "tcms> ";
        string cmd;
        getline(cin, cmd);
        cmd = strings::trim(cmd);
        auto res = handle_command(
                cmd,
                make_tuple(
                        "ls",
                        make_tuple("-l", "pattern", "List (matching) articles"),
                        [&](auto args) {
                            for (auto a: articles) {
                                cout << a->get_name() << '\t';
                            }
                            cout << endl;
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "touch",
                        make_tuple("name", "Create an article"),
                        [&](auto args) {
                            if (args.size() != 2) {
                                cout << "Invalid arguments." << endl;
                                return CommandResult::EMPTY;
                            }
                            if (new_article(args[1])) {
                                return CommandResult::SUCCESS;
                            } else {
                                cout << "duplicated name: " << args[1] << endl;
                                return CommandResult::EMPTY;
                            }
                        }
                ),
                make_tuple(
                        "rm",
                        make_tuple("name", "Delete an article"),
                        [&](auto args) {
                            if (args.size() != 2) {
                                cout << "invalid arguments." << endl;
                                return CommandResult::EMPTY;
                            }
                            if (delete_article(args[1])) {
                                return CommandResult::SUCCESS;
                            } else {
                                cout << "no such article: " << args[1] << endl;
                                return CommandResult::EMPTY;
                            }
                        }
                ),
                make_tuple(
                        "clear",
                        make_tuple("Clear the screen"),
                        [](auto args) {
                            terminal::clear_screen();
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "quit",
                        make_tuple("Exit the program anyway"),
                        [&](auto args) {
                            running = false;
                            return CommandResult::SUCCESS;
                        }
                )
        );
        switch (res) {
            case CommandResult::FAILURE:
                cerr << "Command failed silently." << endl;
                break;
            case CommandResult::NO_MATCH:
                cerr << "Unknown command: " << cmd << endl;
                break;
            case CommandResult::EMPTY:
            case CommandResult::SUCCESS:
                break;
        }
    }
}

void tcms::TCMS::interrupt(int signal) {
    running = false;
    exit(signal);
}

tcms::Article *tcms::TCMS::find_article(const std::string &name) {
    auto iter = std::find_if(articles.begin(), articles.end(), [&](auto a) { return a->get_name() == name; });
    if (iter == articles.end()) {
        return nullptr;
    } else {
        return *iter;
    }
}

bool tcms::TCMS::new_article(const std::string &name) {
    if (find_article(name) != nullptr) {
        return false;
    } else {
        auto a = new Article(name);
        articles.push_back(a);
        a->write_to_file();
        return true;
    }
}

bool tcms::TCMS::delete_article(const std::string &name) {
    auto target = find_article(name);
    if (target == nullptr) {
        return false;
    } else {
        articles.erase(std::find(articles.begin(), articles.end(), target));
        fs::remove_file(target->get_path());
        delete target;
        return true;
    }
}
