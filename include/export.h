#ifndef TCMS_EXPORT_H
#define TCMS_EXPORT_H

#include "Article.h"

namespace tcms {

#define Ostreamable(T, Name) \
        class Name { \
            T *target; \
        public: \
            explicit Name(T *target) : target(target) {} \
            friend std::ostream &operator<<(std::ostream &os, const Name &m); \
        }


    Ostreamable(TitleFrame, MarkdownTitle);

    Ostreamable(ParagraphFrame, MarkdownParagraph);

    Ostreamable(ImageFrame, MarkdownImage);

    Ostreamable(TitleFrame, PlainTitle);

    Ostreamable(ParagraphFrame, PlainParagraph);

    Ostreamable(ImageFrame, PlainImage);

    Ostreamable(TitleFrame, HTMLTitle);

    Ostreamable(ParagraphFrame, HTMLParagraph);

    Ostreamable(ImageFrame, HTMLImage);

    Ostreamable(AuthorTag, PlainAuthorTag);

    Ostreamable(LanguageTag, PlainLanguageTag);

    Ostreamable(TitleTag, PlainTitleTag);

    Ostreamable(AuthorTag, HTMLAuthorTag);

    Ostreamable(LanguageTag, HTMLLanguageTag);

}
#endif //TCMS_EXPORT_H
