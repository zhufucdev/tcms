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
        std::string name;
        id_type id;
        std::vector<FrameGetter> frames;
        Metadata metadata;

        Article(id_type id, const std::string &name, const Metadata &metadata,
                const std::vector<FrameGetter> &frames);

    public:

        Article(const std::string &name);

        ~Article();

        id_type get_id() const;

        std::vector<FrameGetter> get_frames() const;

        Metadata &get_metadata();

        ByteArray serialize() const override;

        static Article *deserialize(ByteArray ba);
    };
}
#endif //TCMS_ARTICLE_H
