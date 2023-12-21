#include "tcms.h"
#include "fs.h"
#include "strings.h"
#include "terminal.h"
#include "find.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace tcms;

/* Initialization */
CliClient::CliClient(Context &ctx) : ctx(ctx) {}

/* Command logic */

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

bool change_work(Context &ctx, Element *we);

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

inline auto cw_command_handler(Context &ctx) {
    return make_tuple(
            "cw",
            make_tuple("name", "Change the working target"),
            [&](auto args, auto &os, auto &es) {
                auto read = terminal::read_name(args);
                if (read.epos == 1) {
                    es << "lacking parameter to element name" << endl;
                    return CommandResult::EMPTY;
                }
                auto article = ctx.get_current_working_element()->resolve(read.name);
                if (article == nullptr) {
                    es << "no such element: " << read.name << endl;
                    return CommandResult::EMPTY;
                } else {
                    return change_work(ctx, article) ? CommandResult::SUCCESS : CommandResult::FAILURE;
                }
            }
    );
}

inline auto cat_command_handler(Context &ctx) {
    return make_tuple(
            "cat",
            make_tuple("name", "--markdown", "--html", "Print an element, optionally in given variant"),
            [&](auto args, auto &os, auto &es) {
                auto read_n = terminal::read_name(args);
                if (read_n.epos < args.size() - 1) {
                    es << "too many arguments" << endl;
                    return CommandResult::EMPTY;
                } else {
                    auto target = ctx.get_current_working_element()->resolve(read_n.name);
                    if (target == nullptr) {
                        es << "no such element: " << read_n.name << endl;
                        return CommandResult::EMPTY;
                    } else {
                        auto read_f = terminal::read_flags(args, read_n.epos);
                        ExportVariant variant = PLAIN;
                        if (read_f.has_named("markdown")) {
                            variant = MARKDOWN;
                        } else if (read_f.has_named("html")) {
                            variant = HTML;
                        }
                        try {
                            target->output(os, variant);
                            return CommandResult::SUCCESS;
                        } catch (const std::exception &e) {
                            es << "Error while reading frame: " << e.what() << endl;
                            return CommandResult::EMPTY;
                        }
                    }
                }
            }
    );
}

inline auto rm_command_handler(Context &ctx) {
    return make_tuple(
            "rm",
            make_tuple("name", "Delete an element"),
            [&](auto args, auto &os, auto &es) {
                if (args.size() < 2) {
                    es << "too few arguments" << endl;
                    return CommandResult::EMPTY;
                }
                auto read = terminal::read_name(args);
                while (true) {
                    auto r = ctx.get_current_working_element()->resolve(read.name);
                    if (r == nullptr) {
                        es << "no such element: " << read.name << endl;
                        return CommandResult::EMPTY;
                    }
                    r->remove();
                    if (read.epos >= args.size()) {
                        break;
                    }
                    read = terminal::read_name(args, read.epos);
                }
                return CommandResult::SUCCESS;
            }
    );
}

inline unsigned char frame_filter_by_char(char c) {
    switch (c) {
        case 'h':
            return FrameType::TITLE;
        case 'p':
            return FrameType::PARAGRAPH;
        case 'i':
            return FrameType::IMAGE;
        case '*':
            return 0xff;
        default:
            return 0;
    }
}

inline unsigned char frame_filter_by_str(const std::string &str) {
    unsigned char r = 0;
    for (auto c: str) {
        r |= frame_filter_by_char(c);
    }
    return r;
}

inline auto ls_command_handler(Context &ctx) {
    return make_tuple(
            "ls",
            make_tuple("pattern", "-l", "-a", "-t type", "-m",
                       "List matching articles, frames or contacts"),
            [&](auto args, auto &os, auto &es) {
                auto read_n = terminal::read_name(args);
                auto target = ctx.get_current_working_element();
                if (read_n.name.length() > 1 && read_n.name[0] != '-') {
                    target = target->resolve(read_n.name);
                }
                auto read_f = terminal::read_flags(args, read_n.epos);
                os << behavior::ListInElement(
                        ctx,
                        target,
                        read_f.has_single('l'),
                        read_f.has_single('a'),
                        frame_filter_by_str(read_f.get_parameter('t', "*"))
                );
                os << endl;
                return CommandResult::SUCCESS;
            }
    );
}

inline auto ln_command_handler(Context &ctx) {
    return make_tuple(
            "ln",
            make_tuple("src", "dst", "Create a linked frame in another article"),
            [&](auto args, auto &os, auto &es) {
                auto read_n = terminal::read_name(args);
                if (read_n.epos == 1) {
                    cerr << "lacking parameter to src" << endl;
                    return CommandResult::EMPTY;
                } else if (read_n.epos >= args.size()) {
                    cerr << "lacking parameter to dst" << endl;
                    return CommandResult::EMPTY;
                }
                auto src_name = read_n.name;
                read_n = terminal::read_name(args, read_n.epos);
                std::string &dst_name = read_n.name;
                auto src = ctx.get_current_working_element()->resolve(src_name),
                        dst = ctx.get_current_working_element()->resolve(dst_name);
                if (src == nullptr || src->get_type() != FRAME) {
                    cerr << "no such frame: " << src_name << endl;
                    return CommandResult::EMPTY;
                }
                if (dst == nullptr || dst->get_type() != ARTICLE) {
                    cerr << "no such article: " << dst_name << endl;
                    return CommandResult::EMPTY;
                }
                auto frame = dynamic_cast<FrameElement *>(src);
                auto article = dynamic_cast<ArticleElement *>(dst);
                article->get()->add_frame(frame->get()->clone());
                return CommandResult::SUCCESS;
            }
    );
}

void tcms::CliClient::event_loop() {
    print_dialog("TCMS - The Content Management System", "Welcome to TCMS. Type ? for help.");
    change_work(ctx, new RootElement(ctx));
}

void tcms::CliClient::interrupt(int signal) {
    ctx.alter_cwe(nullptr);
    exit(signal);
}

bool change_work(Context &ctx, RootElement *ele) {
    while (ctx.get_current_working_element() == ele) {
        if (cin.eof()) {
            ctx.alter_cwe(nullptr);
            break;
        }
        cout << "tcms> ";
        string cmd;
        getline(cin, cmd);
        cmd = strings::trim(cmd);
        auto res = handle_command(
                cmd,
                ls_command_handler(ctx),
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
                                try {
                                    new ArticleElement(read.name, ctx);
                                } catch (const std::exception &e) {
                                    es << e.what() << endl;
                                }
                                if (read.epos >= args.size()) {
                                    break;
                                }
                                read = terminal::read_name(args, read.epos);
                            }
                            return CommandResult::SUCCESS;
                        }
                ),
                rm_command_handler(ctx),
                cw_command_handler(ctx),
                cat_command_handler(ctx),
                ln_command_handler(ctx),
                clear_command_handler(),
                make_tuple(
                        "q",
                        make_tuple("Exit the program anyway"),
                        [&](auto args, auto &os, auto &es) {
                            ctx.alter_cwe(nullptr);
                            return CommandResult::SUCCESS;
                        }
                )
        );
        default_command_result_handler(res, cmd);
    }
    return true;
}

bool change_work(Context &ctx, Article *article, ArticleElement *ele) {
    cout << "\n\n";
    print_dialog(article->get_name(), "Working on this article. Type ? for help.");
    cout << "\n";
    auto working = true;
    auto header = strings::truncate(article->get_name());
    while (ctx.get_current_working_element() == ele) {
        if (cin.eof()) {
            ctx.alter_cwe(nullptr);
            break;
        }
        cout << header << "> ";
        string cmd;
        getline(cin, cmd);
        auto res = handle_command(
                cmd,
                ls_command_handler(ctx),
                cw_command_handler(ctx),
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
                rm_command_handler(ctx),
                cat_command_handler(ctx),
                ln_command_handler(ctx),
                clear_command_handler(),
                make_tuple(
                        "q",
                        make_tuple("Stop working on this article"),
                        [&](auto args, auto &os, auto &es) {
                            ctx.alter_cwe(ele->parent);
                            return CommandResult::SUCCESS;
                        }
                ),
                make_tuple(
                        "q!",
                        make_tuple("Exit the program anyway"),
                        [&](auto args, auto &os, auto &es) {
                            ctx.alter_cwe(nullptr);
                            return CommandResult::SUCCESS;
                        }
                )
        );

        default_command_result_handler(res, cmd);
    }
    return true;
}

bool change_work(Context &ctx, Element *we) {
    ctx.alter_cwe(we);
    if (auto r = dynamic_cast<RootElement *>(we)) {
        return change_work(ctx, r);
    } else if (auto a = dynamic_cast<ArticleElement *>(we)) {
        return change_work(ctx, a->get(), a);
    }
    return false;
}
