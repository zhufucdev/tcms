#include "export.h"
#include "tcms.h"
#include <ostream>

using namespace tcms;
using namespace tcms::behavior;


std::ostream &tcms::operator<<(std::ostream &os, const MarkdownTitle &m) {
    std::string prefix;
    for (int i = 0; i < m.target.get_depth(); ++i) {
        prefix += "#";
    }
    os << prefix << ' ' << m.target.to_string() << '\n';
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const MarkdownParagraph &m) {
    os << m.target.to_string() << "\n\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const MarkdownImage &m) {
    os << "![" << m.target.get_caption() << "](" << fs::path_to_string(m.target.get_image_path()) << ")\n\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const PlainTitle &m) {
    os << m.target.to_string() << '\n';
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const PlainParagraph &m) {
    os << m.target.to_string() << '\n';
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const PlainImage &m) {
    os << "image (" << m.target.get_caption() << ")\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const HTMLTitle &m) {
    int depth = m.target.get_depth();
    os << "<h" << depth << ">" << m.target.to_string() << "</h" << depth << ">\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const HTMLParagraph &m) {
    os << "<p>" << m.target.to_string() << "</p>\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const HTMLImage &m) {
    os << "<img alt=\"" << m.target.get_caption() << "\" src=\""
       << fs::path_to_string(m.target.get_image_path()) << "\"/>\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::PlainAuthorTag &m) {
    os << "Author: " << m.target.to_string() << '\n';
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::PlainLanguageTag &m) {
    os << "Language: " << m.target.to_string() << '\n';
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::PlainTitleTag &m) {
    os << "Title: " << m.target.to_string() << '\n';
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::HTMLAuthorTag &m) {
    os << "<p><b>Author:</b> <i>" << m.target.to_string() << "</i></p>\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::HTMLLanguageTag &m) {
    os << "<p><b>Language:</b> <i>" << m.target.to_string() << "</i></p>\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::HTMLTitleTag &m) {
    os << "<p><b>Title:</b> <i>" << m.target.to_string() << "</i></p>\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::MarkdownAuthorTag &m) {
    os << "**Author:** __" << m.target.to_string() << "__\n\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::MarkdownLanguageTag &m) {
    os << "**Language:** __" << m.target.to_string() << "__\n\n";
    return os;
}

std::ostream &tcms::operator<<(std::ostream &os, const tcms::MarkdownTitleTag &m) {
    os << "**Title:** __" << m.target.to_string() << "__\n\n";
    return os;
}

#define OStreamifyFrameEle(Name, Title, Para, Image) \
std::ostream &tcms::operator<<(std::ostream &os, const Name &m) { \
    auto getter = m.target.get(); \
    switch (getter->get_type()) { \
        case HEADER: \
            os << Title(*dynamic_cast<TitleFrame *>(getter->get())); \
            break; \
        case PARAGRAPH: \
            os << Para(*dynamic_cast<ParagraphFrame *>(getter->get())); \
            break; \
        case IMAGE: \
            os << Image(*dynamic_cast<ImageFrame *>(getter->get())); \
            break; \
    } \
    return os; \
}

OStreamifyFrameEle(MarkdownFrameElement, MarkdownTitle, MarkdownParagraph, MarkdownImage)

OStreamifyFrameEle(PlainFrameElement, PlainTitle, PlainParagraph, PlainImage)

OStreamifyFrameEle(HTMLFrameElement, HTMLTitle, HTMLParagraph, HTMLImage)

#define OStreamifyTagEle(Name, Title, Author, Language) \
std::ostream &tcms::operator<<(std::ostream &os, const tcms::Name &m) { \
    auto tag = m.target.get(); \
    switch (tag->get_type()) { \
        case LANG: \
            os << Language(*dynamic_cast<LanguageTag *>(tag)); \
            break; \
        case AUTHOR: \
            os << Author(*dynamic_cast<AuthorTag *>(tag)); \
            break; \
        case TITLE: \
            os << Title(*dynamic_cast<TitleTag *>(tag)); \
            break; \
    } \
    return os; \
}

OStreamifyTagEle(MarkdownTagElement, MarkdownTitleTag, MarkdownAuthorTag, MarkdownLanguageTag)

OStreamifyTagEle(PlainTagElement, PlainTitleTag, PlainAuthorTag, PlainLanguageTag)

OStreamifyTagEle(HTMLTagElement, HTMLTitleTag, HTMLAuthorTag, HTMLLanguageTag)
