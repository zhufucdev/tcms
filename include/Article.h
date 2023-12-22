#ifndef TCMS_ARTICLE_H
#define TCMS_ARTICLE_H

#include "bytes.h"
#include "increment.h"
#include "frame.h"
#include "metadata.h"
#include "FrameGetter.h"

namespace tcms {
    class Article : public fs::FileAssociated {
    private:
        std::string name;
        id_type id;
        std::vector<FrameGetter *> frames;
        Metadata metadata;

        Article(id_type id, const std::string &name, const Metadata &metadata,
                const std::vector<FrameGetter *> &frames);

    public:

        Article(const std::string &name);

        ~Article();

        id_type get_id() const;

        std::string get_name() const;

        fs::Path get_path() const override;

        std::vector<FrameGetter *> get_frames() const;

        void add_frame(Frame *frame, id_type after = -1);

        void add_frame(FrameGetter *getter, id_type after = -1);

        void remove_frame(FrameGetter *frame);

        Metadata &get_metadata();

        ByteArray serialize() const override;

        static Article *deserialize(ByteArray ba);

        void remove() override;

        bool operator==(const Article &a) const;
    };
}
#endif //TCMS_ARTICLE_H
