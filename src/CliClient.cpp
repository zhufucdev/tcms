#include "tcms.h"
#include "fs.h"
#include "strhelper.h"
#include "terminal.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace tcms;

CliClient::CliClient(Context &ctx) : ctx(ctx) {}

enum CommandResult {
    SUCCESS = 0,
    FAILURE,
    EMPTY,
    NO_MATCH
};

template<typename Handler>
CommandResult command_file_pipe(const vector<string> &args, ostream &os, ostream &es, const Handler &handler) {
    int pip_pos;
    unsigned char mode;
    for (pip_pos = 0; pip_pos < args.size(); ++pip_pos) {
        if (args[pip_pos] == ">") {
            mode = 0;
            break;
        } else if (args[pip_pos] == ">>") {
            mode = 1;
            break;
        } else if (args[pip_pos] == "<" || args[pip_pos] == "<<") {
            mode = 2;
            break;
        }
    }
    if (pip_pos >= args.size()) {
        // not piped
        return handler(args, os, es);
    } else if (pip_pos == args.size() - 1) {
        cerr << "invalid pipeline syntax";
        return CommandResult::EMPTY;
    } else {
        auto direction = terminal::read_name(args, pip_pos + 1);
        if (direction.epos != args.size()) {
            cerr << "warning: ignoring redundant parameters after pipeline direction" << endl;
        }
        if (mode <= 1) {
            // output stream
            ofstream ofs(direction.name, ofstream::out | (mode == 1 ? ofstream::app : ofstream::trunc));
            vector<std::string> sliced_args(args.begin(), args.begin() + pip_pos);
            auto res = handler(sliced_args, ofs, cerr);
            ofs.close();
            return res;
        } else {
            // input stream
            try {
                auto ba = fs::read_file(fs::string_to_path(direction.name));
                auto effective_args = args;
                effective_args.erase(effective_args.begin() + pip_pos, effective_args.end());
                for (const auto &new_arg: terminal::read_args(ba.content)) {
                    effective_args.push_back(new_arg);
                }
                return handler(effective_args, cout, cerr);
            } catch (const std::exception &e) {
                es << e.what() << endl;
                return CommandResult::EMPTY;
            }
        }
    }
}

template<typename Handler>
CommandResult handle_command(const vector<string> &args, ostream &os, ostream &es, const Handler &last_handler) {
    if (args[0] == get<0>(last_handler)) {
        return command_file_pipe(args, os, es, get<2>(last_handler));
    } else {
        return CommandResult::NO_MATCH;
    }
}

template<typename Handler, typename ...Handlers>
CommandResult handle_command(const vector<string> &args, ostream &os, ostream &es, const Handler &first_handler,
                             const Handlers &...handlers) {
    if (args.empty()) {
        return CommandResult::EMPTY;
    } else if (get<0>(first_handler) == args[0]) {
        return command_file_pipe(args, os, es, get<2>(first_handler));
    } else {
        return handle_command(args, os, es, handlers...);
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
    auto args = terminal::read_args(input);
    if (args.size() == 1 && args[0] == "?") {
        cout << "\nHelp:\n\n";
        print_help(first_handler, handlers...);
        cout << endl;
        return CommandResult::SUCCESS;
    } else {
        int pipe_pos = 0;
        stringstream buf;
        CommandResult last_res = CommandResult::SUCCESS;
        while (pipe_pos < args.size() && last_res == CommandResult::SUCCESS) {
            for (; pipe_pos < args.size() && args[pipe_pos] != "|"; ++pipe_pos);
            buf = stringstream();
            if (pipe_pos < args.size()) {
                last_res = handle_command(vector<string>(args.begin(), args.begin() + pipe_pos), buf,
                                          cerr, first_handler, handlers...);
                args = vector<string>(args.begin() + pipe_pos + 1, args.end());
                for (pipe_pos = 0; pipe_pos < args.size() && args[pipe_pos] != "|"; ++pipe_pos);
                auto dash_iter = std::find(args.begin(), args.end(), "-");
                if (std::distance(args.begin(), dash_iter) >= pipe_pos) {
                    for (const auto &arg: terminal::read_args(buf.str())) {
                        args.insert(args.begin() + pipe_pos, arg);
                        pipe_pos++;
                    }
                    pipe_pos--;
                } else {
                    auto cp = vector<string>(args.begin(), dash_iter);
                    for (const auto &arg: terminal::read_args(buf.str())) {
                        cp.push_back(arg);
                    }
                    for (dash_iter++; dash_iter != args.end(); dash_iter++) {
                        cp.push_back(*dash_iter);
                    }
                    args = cp;
                }
            } else {
                last_res = handle_command(args, buf, cerr, first_handler, handlers...);
            }
        }
        cout << buf.str();
        return last_res;
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

inline auto quit_command_handler(Context &ctx) {
    return make_tuple(
            "q",
            make_tuple("Stop working on this element"),
            [&](auto args, auto &os, auto &es) {
                ctx.alter_cwe(ctx.get_current_working_element()->parent);
                return CommandResult::SUCCESS;
            }
    );
}

inline auto quit_anyway_command_handler(Context &ctx) {
    return make_tuple(
            "q!",
            make_tuple("Exit the program anyway"),
            [&](auto args, auto &os, auto &es) {
                ctx.alter_cwe(nullptr);
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
                auto ele = ctx.get_current_working_element()->resolve(read.name);
                if (ele == nullptr) {
                    es << "no such element: " << read.name << endl;
                    return CommandResult::EMPTY;
                } else {
                    return change_work(ctx, ele) ? CommandResult::SUCCESS : CommandResult::FAILURE;
                }
            }
    );
}

inline auto cat_command_handler(Context &ctx) {
    return make_tuple(
            "cat",
            make_tuple("name...", "-m --markdown", "-h --html", "Print an element, optionally in given variant"),
            [&](auto args, auto &os, auto &es) {
                int flag_offset = 1;
                for (int i = 1; i < args.size(); ++i) {
                    if (terminal::is_flag(args[i])) {
                        flag_offset = i;
                    }
                }
                auto read_f = terminal::read_flags(args, flag_offset);
                auto read_n = terminal::read_name(args, flag_offset == 1 ? read_f.epos : 1);
                bool printed = false;

                while (true) {
                    if (!printed || !read_n.name.empty()) {
                        auto target = ctx.get_current_working_element()->resolve(read_n.name);
                        if (target == nullptr) {
                            es << "no such element: " << read_n.name << endl;
                            return CommandResult::EMPTY;
                        } else {
                            ExportVariant variant = PLAIN;
                            if (read_f.has_single('m') || read_f.has_named("markdown")) {
                                variant = MARKDOWN;
                            } else if (read_f.has_single('h') || read_f.has_named("html")) {
                                variant = HTML;
                            }
                            try {
                                target->output(os, variant);
                            } catch (const std::exception &e) {
                                es << "Error while reading frame: " << e.what() << endl;
                                return CommandResult::EMPTY;
                            }
                            printed = true;
                        }
                    }
                    if (read_n.epos >= args.size()) {
                        break;
                    }
                    do {
                        read_n = terminal::read_name(args, read_n.epos);
                    } while (terminal::is_flag(read_n.name));
                }
                return CommandResult::SUCCESS;
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
                    if (read.name.empty()) {
                        continue;
                    }
                    auto r = ctx.get_current_working_element()->resolve(read.name);
                    if (r == nullptr) {
                        es << "no such element: " << read.name << endl;
                        return CommandResult::EMPTY;
                    }
                    try {
                        r->remove();
                    } catch (const std::exception &e) {
                        es << "error while removing " << read.name << ": " << e.what() << endl;
                        return CommandResult::EMPTY;
                    }
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
            return FrameType::HEADER;
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
            make_tuple("name", "-l", "-a", "-t type", "-m",
                       "List matching articles, frames or contacts"),
            [&](auto args, auto &os, auto &es) {
                Element *target = nullptr;
                int flags_offset = 1;
                for (int i = 1; i < args.size(); ++i) {
                    if (terminal::is_flag(args[i])) {
                        flags_offset = i;
                        break;
                    }
                }
                auto printed = false;
                auto read_f = terminal::read_flags(args, flags_offset);
                auto read_n = terminal::read_name(args);
                while (true) {
                    while (terminal::is_flag(read_n.name)) {
                        read_n = terminal::read_name(args, read_n.epos);
                    }
                    if (read_n.epos >= args.size() && printed) {
                        break;
                    } else {
                        target = ctx.get_current_working_element()->resolve(read_n.name);
                    }
                    if (target == nullptr) {
                        es << "no such element" << endl;
                        return CommandResult::EMPTY;
                    }
                    os << behavior::ListInElement(
                            ctx,
                            target,
                            read_f.has_single('l'),
                            read_f.has_single('a'),
                            frame_filter_by_str(read_f.get_parameter('t', "*"))
                    );
                    os << endl;
                    if (read_n.epos >= args.size()) {
                        break;
                    }
                    printed = true;
                    read_n = terminal::read_name(args, read_n.epos);
                }
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

template<typename Capture, typename Search>
inline auto find_command_handler(Capture capture, const string &prompt, Search s) {
    return make_tuple(
            "find",
            make_tuple("keyword", "-i --ignoring-case", "-r --regex", "-s --source", prompt),
            [&](auto args, auto &os, auto &es) {
                auto read_n = terminal::read_name(args);
                if (read_n.epos == 1) {
                    es << "lacking argument to keyword" << endl;
                    return CommandResult::EMPTY;
                }
                auto read_f = terminal::read_flags(args, read_n.epos);
                auto ic = read_f.has_single('i') || read_f.has_named("ignoring-case"),
                        regex = read_f.has_single('r') || read_f.has_named("regex"),
                        source = read_f.has_single('s') || read_f.has_named("source");
                try {
                    s(capture, os, read_n.name, ic, regex, source);
                } catch (const std::runtime_error &e) {
                    es << "error while searching: " << e.what() << endl;
                }
                return CommandResult::SUCCESS;
            }
    );
}

inline auto echo_command_handler() {
    return make_tuple(
            "echo",
            make_tuple("--tab-break", "params...", "Repeat the params with additional breaks"),
            [&](auto args, auto &os, auto &es) {
                auto read_f = terminal::read_flags(args);
                auto params = terminal::read_paragraph(args, read_f.epos);
                os << params;
                if (read_f.has_named("tab-break")) {
                    os << '\t';
                } else {
                    os << '\n';
                }
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
                find_command_handler<Context &>(
                        ctx, "Search for an article",
                        [&](auto &ctx, auto &os, auto &k, auto ic, auto r, auto s) {
                            for (auto const &article: ctx.articles) {
                                auto name = article->get_name();
                                if (strings::match(name, k, ic, r)) {
                                    os << name << '\n';
                                } else {
                                    for (auto tag: article->get_metadata().get_tags()) {
                                        if (strings::match(tag->to_string(), k, ic, r)) {
                                            auto ele = TagElement(tag, ctx);
                                            os << name;
                                            if (s) {
                                                os << " (" << PlainTagElement(ele) << ")";
                                            }
                                            os << "\n";
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                ),
                cat_command_handler(ctx),
                ln_command_handler(ctx),
                echo_command_handler(),
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
                        make_tuple("-d depth", "-A after", "title...", "Append a title, optionally with depth"),
                        [&](auto args, auto &os, auto &es) {
                            auto read_f = terminal::read_flags(args);
                            int depth = strings::parse_number<int>(read_f.get_parameter('d', "1")),
                                    after = strings::parse_number<int>(read_f.get_parameter('A', "-1"));
                            if (depth <= 0 || depth >= 0xff) {
                                es << "invalid depth" << endl;
                                return CommandResult::EMPTY;
                            }

                            if (args.size() <= read_f.epos) {
                                es << "lack parameter to title content" << endl;
                                return CommandResult::EMPTY;
                            }

                            try {
                                article->add_frame(
                                        new TitleFrame(terminal::read_paragraph(args, read_f.epos), depth),
                                        after
                                );
                                return CommandResult::SUCCESS;
                            } catch (const std::runtime_error &e) {
                                es << e.what() << endl;
                                return CommandResult::EMPTY;
                            }
                        }
                ),
                make_tuple(
                        "p",
                        make_tuple("-A after", "paragraph...", "Append a paragraph"),
                        [&](auto args, auto &os, auto &es) {
                            auto read_f = terminal::read_flags(args);
                            if (read_f.epos >= args.size()) {
                                es << "lack parameter to paragraph content" << endl;
                                return CommandResult::EMPTY;
                            } else {
                                int after = strings::parse_number<int>(read_f.get_parameter('A', "-1"));
                                try {
                                    article->add_frame(new ParagraphFrame(terminal::read_paragraph(args, read_f.epos)),
                                                       after);
                                    return CommandResult::SUCCESS;
                                } catch (const std::runtime_error &e) {
                                    es << e.what() << endl;
                                    return CommandResult::EMPTY;
                                }
                            }
                        }
                ),
                make_tuple(
                        "i",
                        make_tuple("-A after", "caption", "file", "Append an image"),
                        [&](auto args, auto &os, auto &es) {
                            auto read_f = terminal::read_flags(args);
                            auto read_n = terminal::read_name(args, read_f.epos);
                            if (read_n.epos == read_f.epos) {
                                es << "lack parameter to image caption" << endl;
                                return CommandResult::EMPTY;
                            } else {
                                auto read_nf = terminal::read_name(args, read_n.epos);
                                if (read_nf.epos == read_n.epos) {
                                    es << "lack parameter to image file" << endl;
                                    return CommandResult::EMPTY;
                                } else {
                                    auto i = new ImageFrame(read_n.name);
                                    try {
                                        i->set_file(fs::string_to_path(read_nf.name));
                                    } catch (const std::runtime_error &e) {
                                        es << e.what() << endl;
                                        return CommandResult::EMPTY;
                                    }
                                    auto after = strings::parse_number<int>(read_f.get_parameter('A', "-1"));
                                    try {
                                        article->add_frame(i, after);
                                    } catch (const std::runtime_error &e) {
                                        es << e.what() << endl;
                                        return CommandResult::EMPTY;
                                    }
                                }
                            }
                            return CommandResult::SUCCESS;
                        }
                ),
                rm_command_handler(ctx),
                cat_command_handler(ctx),
                ln_command_handler(ctx),
                find_command_handler<Article &>(
                        *article, "Search for a frame",
                        [&](auto &a, auto &os, auto &k, auto ic, auto r, auto s) {
                            for (auto f: a.get_frames()) {
                                if (strings::match(f->get()->to_string(), k, ic, r)) {
                                    os << f->get_id() << '\t';
                                }
                            }
                            os << '\n';
                        }
                ),
                echo_command_handler(),
                clear_command_handler(),
                quit_command_handler(ctx),
                quit_anyway_command_handler(ctx)
        );

        default_command_result_handler(res, cmd);
    }
    return true;
}

void save_within_article(Article *a, ostream &es) {
    if (a != nullptr) {
        a->write_to_file();
    } else {
        es << "warning: orphaned metadata element, your progress will not be saved" << endl;
    }
}

bool change_work(Context &ctx, Metadata &metadata, MetadataElement *ele) {
    cout << "\n\n";

    Article *parent = nullptr;

    {
        stringstream body;
        if (auto a = dynamic_cast<ArticleElement *>(ele->parent)) {
            parent = a->get();
            body << "Working on metadata of " << a->get()->get_name() << ". ";
        } else {
            body << "Working on this metadata. ";
        }
        body << "Type ? for help.";
        print_dialog("metadata", body.str());
        cout << "\n";
    }

    while (ctx.get_current_working_element() == ele) {
        if (cin.eof()) {
            ctx.alter_cwe(nullptr);
            break;
        }
        cout << "metadata> ";
        string cmd;
        getline(cin, cmd);

        auto res = handle_command(
                cmd,
                ls_command_handler(ctx),
                cw_command_handler(ctx),
                make_tuple(
                        "l",
                        make_tuple("name", "Create a language tag"),
                        [&](auto args, auto &os, auto &es) {
                            auto read_n = terminal::read_name(args);
                            if (read_n.epos == 1) {
                                es << "too many arguments" << endl;
                                return CommandResult::EMPTY;
                            }
                            try {
                                Language lang = Language::parse(read_n.name);
                                metadata.add_tag(new LanguageTag(lang));
                                save_within_article(parent, es);
                                return CommandResult::SUCCESS;
                            } catch (const runtime_error &e) {
                                es << e.what() << endl;
                                return CommandResult::EMPTY;
                            }
                        }
                ),
                make_tuple(
                        "a",
                        make_tuple("-l name", "names...", "Create or link an author tag"),
                        [&](auto args, auto &os, auto &es) {
                            auto linking = terminal::read_flags(args).get_parameter('l', "");
                            if (!linking.empty()) {
                                auto linker_tag = ele->resolve(linking);
                                if (auto te = dynamic_cast<TagElement *>(linker_tag)) {
                                    if (auto at = dynamic_cast<AuthorTag *>(te->get())) {
                                        auto tag = new AuthorTag(at->get_author()->clone());
                                        metadata.add_tag(tag);
                                        save_within_article(parent, es);
                                        return CommandResult::SUCCESS;
                                    }
                                }
                                es << linking << " is not an author tag" << endl;
                                return CommandResult::EMPTY;
                            } else {
                                auto read_n = terminal::read_name(args);
                                if (read_n.epos == 1) {
                                    es << "lacking parameter to names" << endl;
                                    return CommandResult::EMPTY;
                                }
                                auto getter = new MemoryContactGetter(new Contact(read_n.name));
                                auto tag = new AuthorTag(getter);
                                int i = 0;
                                while (read_n.epos < args.size()) {
                                    read_n = terminal::read_name(args, read_n.epos);
                                    getter->get()->set_name(++i, read_n.name);
                                }
                                getter->write_to_file();
                                metadata.add_tag(tag);
                                save_within_article(parent, es);
                                return CommandResult::SUCCESS;
                            }
                        }
                ),
                make_tuple(
                        "t",
                        make_tuple("title...", "Create a title tag"),
                        [&](auto args, auto &os, auto &es) {
                            auto title = terminal::read_paragraph(args);
                            metadata.add_tag(new TitleTag(title));
                            save_within_article(parent, es);
                            return CommandResult::SUCCESS;
                        }
                ),
                rm_command_handler(ctx),
                cat_command_handler(ctx),
                find_command_handler(
                        make_tuple<Metadata &, Context &>(metadata, ctx), "Find a tag",
                        [&](auto &m, auto &os, auto &k, auto ic, auto r, auto s) {
                            auto tags = get<0>(m).get_tags();
                            for (int i = 0; i < tags.size(); ++i) {
                                if (strings::match(tags[i]->to_string(), k, ic, r)) {
                                    if (s) {
                                        auto ele = TagElement(tags[i], get<1>(m));
                                        os << PlainTagElement(ele) << '\n';
                                    } else {
                                        os << i << '\t';
                                    }
                                }
                            }
                            if (!s) {
                                os << '\n';
                            }
                        }
                ),
                echo_command_handler(),
                clear_command_handler(),
                quit_command_handler(ctx),
                quit_anyway_command_handler(ctx)
        );

        default_command_result_handler(res, cmd);
    }
    return true;
}

bool change_work(Context &ctx, Element *we) {
    if (auto r = dynamic_cast<RootElement *>(we)) {
        ctx.alter_cwe(we);
        return change_work(ctx, r);
    } else if (auto a = dynamic_cast<ArticleElement *>(we)) {
        ctx.alter_cwe(we);
        return change_work(ctx, a->get(), a);
    } else if (auto m = dynamic_cast<MetadataElement *>(we)) {
        ctx.alter_cwe(we);
        return change_work(ctx, m->get(), m);
    }
    return false;
}
