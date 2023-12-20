#ifndef TCMS_ARTICLE_H
#define TCMS_ARTICLE_H

#include "bytes.h"
#include "increment.h"
#include "content.h"
#include "metadata.h"
#include "FrameGetter.h"

namespace tcms {
    class Article : public bytes::BinarySerializable {
    private:
        id_type id;
        std::vector<FrameGetter> frames;
        Metadata metadata;
    public:
        explicit Article(id_type id, const Metadata &metadata, const std::vector<FrameGetter> &frames);

        ~Article();

        id_type get_id() const;

        std::vector<FrameGetter> get_frames() const;

        Metadata &get_metadata();

        ByteArray serialize() const override;

        static Article *deserialize(ByteArray ba);
    };
}
#endif //TCMS_ARTICLE_H
