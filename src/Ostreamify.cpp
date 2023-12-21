#include "tcms.h"
#include <ostream>

using namespace tcms;
using namespace tcms::behavior;


std::ostream &tcms::behavior::operator<<(std::ostream &os, const MarkdownTitle &m) {
    std::string prefix;
    for (int i = 0; i < m.target->get_depth(); ++i) {
        prefix += "#";
    }
    os << prefix << ' ' << m.target->to_string() << '\n';
    return os;
}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const MarkdownParagraph &m) {
    os << m.target->to_string() << '\n';
    return os;
}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const MarkdownImage &m) {
    os << "![" << m.target->get_caption() << "](" << fs::path_to_string(m.target->get_image_path()) << ")\n";
    return os;
}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const PlainTitle &m) {
    os << m.target->to_string() << '\n';
    return os;
}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const PlainParagraph &m) {
    os << m.target->to_string() << '\n';
    return os;
}

std::ostream &tcms::behavior::operator<<(std::ostream &os, const PlainImage &m) {
    os << "image (" << m.target->get_caption() << ")\n";
    return os;
}

#define OStreamifyArticle(Name, Title, Para, Image) \
std::ostream &tcms::behavior::operator<<(std::ostream &os, const Name &m) {\
    for (auto const &f: m.target->get_frames()) {\
        switch (f->get_type()) {\
            case TITLE:\
                os << Title(m.ctx, dynamic_cast<TitleFrame *>(f->get()));\
                break;\
            case PARAGRAPH:\
                os << Para(m.ctx, dynamic_cast<ParagraphFrame *>(f->get()));\
                break;\
            case IMAGE:\
                os << Image(m.ctx, dynamic_cast<ImageFrame *>(f->get()));\
                break;\
        }\
    }\
    return os;\
}

#define OStreamifyFrameEle(Name, Title, Para, Image) \
std::ostream &tcms::behavior::operator<<(std::ostream &os, const Name &m) { \
    auto getter = m.target->get(); \
    switch (getter->get_type()) { \
        case TITLE: \
            os << Title(m.ctx, dynamic_cast<TitleFrame *>(getter->get())); \
            break; \
        case PARAGRAPH: \
            os << Para(m.ctx, dynamic_cast<ParagraphFrame *>(getter->get())); \
            break; \
        case IMAGE: \
            os << Image(m.ctx, dynamic_cast<ImageFrame *>(getter->get())); \
            break; \
    } \
    return os; \
}

OStreamifyArticle(MarkdownArticle, MarkdownTitle, MarkdownParagraph, MarkdownImage)

OStreamifyArticle(PlainArticle, PlainTitle, PlainParagraph, PlainImage)

OStreamifyFrameEle(MarkdownFrameElement, MarkdownTitle, MarkdownParagraph, MarkdownImage)

OStreamifyFrameEle(PlainFrameElement, PlainTitle, PlainParagraph, PlainImage)
