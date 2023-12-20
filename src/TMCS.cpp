#include "TCMS.h"
#include "fs.h"
#include "strings.h"
#include "terminal.h"
#include "find.h"
#include <iostream>
#include <fstream>

using namespace std;

tcms::TCMS::TCMS() : running(false), articles() {
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
CommandResult command_pipelined(const vector<string> &args, const Handler &handler) {
    int pip_pos;
    bool append_mode;
    for (pip_pos = 0; pip_pos < args.size(); ++pip_pos) {
        if (args[pip_pos] == ">") {
            append_mode = false;
            break;
        } else if (args[pip_pos] == ">>") {
            append_mode = true;
            break;
        }
    }
    if (pip_pos >= args.size()) {
        // not piped
        return handler(args, cout, cerr);
    } else if (pip_pos == args.size() - 1) {
        cerr << "invalid pipeline syntax";
        return CommandResult::EMPTY;
    } else {
        auto direction = terminal::read_name(args, pip_pos + 1);
        if (direction.epos != args.size()) {
            cerr << "warning: ignoring redundant parameters after pipeline direction" << endl;
        }
        ofstream ofs(direction.name, ofstream::out | (append_mode ? ofstream::app : ofstream::trunc));
        vector<std::string> sliced_args(args.begin(), args.begin() + pip_pos);
        auto res = handler(sliced_args, ofs, cerr);
        ofs.close();
        return res;
    }
}

template<typename Handler>
CommandResult handle_command(const vector<string> &args, const Handler &last_handler) {
    if (args[0] == get<0>(last_handler)) {
        return command_pipelined(args, get<2>(last_handler));
    } else {
        return CommandResult::NO_MATCH;
    }
}

template<typename Handler, typename ...Handlers>
CommandResult handle_command(const vector<string> args, const Handler &first_handler, const Handlers &...handlers) {
    if (args.empty()) {
        return CommandResult::EMPTY;
    } else if (get<0>(first_handler) == args[0]) {
        return command_pipelined(args, get<2>(first_handler));
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

inline void default_command_result_handler(CommandResult res, const string &cmd) {
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

inline auto clear_command_handler() {
    return make_tuple(
            "clear",
            make_tuple("Clear the screen"),
            [](auto args, auto &os, auto &es) {
                terminal::clear_screen();
                return CommandResult::SUCCESS;
            }
    );
}

void tcms::TCMS::event_loop() {
    running = true;
    print_dialog("TCMS - The Content Management System", "Welcome to TCMS. Type ? for help.");
    while (running) {
        if (cin.eof()) {
            running = false;
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
                        make_tuple("-l", "-t type", "pattern", "List (matching) articles, frames or contacts"),
                        [&](auto args, auto &os, auto &es) {
                            for (auto a: articles) {
                                os << a->get_name() << '\t';
                            }
                            cout << endl;
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "touch",
                        make_tuple("name", "Create an article"),
                        [&](auto args, auto &os, auto &es) {
                            if (args.size() < 2) {
                                os << "too few arguments" << endl;
                                return CommandResult::EMPTY;
                            }
                            auto read = terminal::read_name(args);
                            while (true) {
                                if (!new_article(read.name)) {
                                    os << "duplicated name: " << args[1] << endl;
                                    return CommandResult::EMPTY;
                                }
                                if (read.epos >= args.size()) {
                                    break;
                                }
                                read = terminal::read_name(args, read.epos);
                            }
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "rm",
                        make_tuple("name", "Delete an article"),
                        [&](auto args, auto &os, auto &es) {
                            if (args.size() < 2) {
                                es << "too few arguments" << endl;
                                return CommandResult::EMPTY;
                            }
                            auto read = terminal::read_name(args);
                            while (true) {
                                if (!delete_article(read.name)) {
                                    es << "no such article: " << read.name << endl;
                                    return CommandResult::EMPTY;
                                }
                                if (read.epos >= args.size()) {
                                    break;
                                }
                                read = terminal::read_name(args, read.epos);
                            }
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "cw",
                        make_tuple("name", "-t type", "Change the working target (of specific type)"),
                        [&](auto args, auto &os, auto &es) {
                            auto read = terminal::read_name(args);
                            if (read.epos == 1) {
                                es << "lacking parameter to article name" << endl;
                                return CommandResult::EMPTY;
                            }
                            auto article = find_article(read.name);
                            if (article == nullptr) {
                                es << "no such article: " << read.name << endl;
                                return CommandResult::EMPTY;
                            } else {
                                return change_work(article) ? CommandResult::SUCCESS : CommandResult::FAILURE;
                            }
                        }
                ),
                make_tuple(
                        "cat",
                        make_tuple("name", "-t type", "Print a target (of specific type)"),
                        [&](auto args, auto &os, auto &es) {
                            auto read = terminal::read_name(args);
                            if (read.epos < args.size()) {
                                es << "too many arguments" << endl;
                                return CommandResult::EMPTY;
                            }
                            return CommandResult::EMPTY;
                        }
                ),
                clear_command_handler(),
                make_tuple(
                        "q",
                        make_tuple("Exit the program anyway"),
                        [&](auto args, auto &os, auto &es) {
                            running = false;
                            return CommandResult::SUCCESS;
                        }
                )
        );
        default_command_result_handler(res, cmd);
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

bool tcms::TCMS::change_work(tcms::Article *article) {
    cout << "\n\n";
    print_dialog(article->get_name(), "Working on this article. Type ? for help.");
    cout << "\n";
    auto working = true;
    auto header = strings::truncate(article->get_name());
    while (running && working) {
        if (cin.eof()) {
            running = false;
            break;
        }
        cout << header << "> ";
        string cmd;
        getline(cin, cmd);
        auto res = handle_command(
                cmd,
                make_tuple(
                        "ls",
                        make_tuple("-t type", "-l", "List frames (of specific type)"),
                        [&](auto args, auto &os, auto &es) {
                            for (auto f: article->get_frames()) {
                                os << f->get_id() << '\t';
                            }
                            os << endl;
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "h",
                        make_tuple("-d depth", "title...", "Append a title (with depth)"),
                        [&](auto args, auto &os, auto &es) {
                            int depth = 1;
                            vector<string>::size_type offset = 1;
                            if (args[1] == "-d") {
                                if (args.size() < 3) {
                                    es << "lacking parameter to depth";
                                    return CommandResult::EMPTY;
                                }
                                depth = strings::parse_number<int>(args[2]);
                                offset = 3;
                            }
                            if (args.size() <= offset) {
                                es << "lack parameter to title content" << endl;
                                return CommandResult::EMPTY;
                            }

                            article->add_frame(new TitleFrame(terminal::read_paragraph(args, offset), depth));
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "p",
                        make_tuple("paragraph...", "Append a paragraph"),
                        [&](auto args, auto &os, auto &es) {
                            if (args.size() < 2) {
                                es << "lack parameter to paragraph content" << endl;
                                return CommandResult::EMPTY;
                            } else {
                                article->add_frame(new ParagraphFrame(terminal::read_paragraph(args)));
                                return CommandResult::SUCCESS;
                            }
                        }
                ),
                make_tuple(
                        "i",
                        make_tuple("caption", "file", "Append an image"),
                        [&](auto args, auto &os, auto &es) {
                            auto read = terminal::read_name(args);
                            if (read.epos >= args.size()) {
                                article->add_frame(new ImageFrame(read.name));
                            } else if (read.epos > 1) {
                                auto i = new ImageFrame(read.name);
                                read = terminal::read_name(args, read.epos);
                                try {
                                    i->set_file(fs::string_to_path(read.name));
                                } catch (const std::runtime_error &e) {
                                    es << e.what() << endl;
                                    return CommandResult::EMPTY;
                                }
                                article->add_frame(i);
                            } else {
                                es << "lack parameter to image caption";
                                return CommandResult::EMPTY;
                            }
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "cat",
                        make_tuple("name", "Print a frame"),
                        [&](auto args, auto &os, auto &es) {
                            auto read = terminal::read_name(args);
                            if (read.epos < args.size() - 1) {
                                es << "too many arguments" << endl;
                                return CommandResult::EMPTY;
                            } else {
                                auto id = strings::parse_number<id_type>(read.name);
                                auto frames = article->get_frames();
                                auto frame = find::by_id<FrameGetter *>(frames.begin(), frames.end(), id);
                                if (frame == nullptr) {
                                    es << "no such frame: " << read.name << endl;
                                    return CommandResult::EMPTY;
                                } else {
                                    try {
                                        os << frame->get()->to_string() << endl;
                                        return CommandResult::SUCCESS;
                                    } catch (const std::exception &e) {
                                        es << "Error while reading frame: " << e.what() << endl;
                                        return CommandResult::EMPTY;
                                    }
                                }
                            }
                        }
                ),
                clear_command_handler(),
                make_tuple(
                        "q",
                        make_tuple("Stop working on this article"),
                        [&](auto args, auto &os, auto &es) {
                            working = false;
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "q!",
                        make_tuple("Exit the program anyway"),
                        [&](auto args, auto &os, auto &es) {
                            running = false;
                            return CommandResult::SUCCESS;
                        }
                )
        );

        default_command_result_handler(res, cmd);
    }
    return true;
}

bool tcms::TCMS::change_work(tcms::Frame *frame) {
    return false;
}

bool tcms::TCMS::change_work(tcms::Contact *contact) {
    return false;
}