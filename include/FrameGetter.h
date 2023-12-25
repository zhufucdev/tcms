//
// Created by Steve Reed on 2023/12/19.
//

#ifndef TCMS_FRAMEGETTER_H
#define TCMS_FRAMEGETTER_H

#include "increment.h"
#include "frame.h"
#include <map>

namespace tcms {
    class FrameGetter : public fs::FileAssociated {
        id_type id;
        FrameType type;

    protected:
        static std::map<id_type, Frame *> cache;
        static std::map<id_type, size_t> rc;

    public:
        FrameGetter(id_type id, FrameType type);

        FrameGetter(const FrameGetter &f);

        ~FrameGetter();

        fs::Path get_path() const override;

        void remove() override;

        virtual Frame *get() const;

        virtual FrameGetter *clone() const;

        FrameType get_type() const;

        id_type get_id() const;

        ByteArray serialize() const override;

        size_t estimated_size() const;

        static FrameGetter *from_file(id_type id);

        static fs::Path get_path(id_type id);
    };

    class MemoryFrameGetter : public FrameGetter {
        Frame *frame;
    public:
        explicit MemoryFrameGetter(Frame *frame);

        Frame *get() const override;

        FrameGetter * clone() const override;
    };
}


#endif //TCMS_FRAMEGETTER_H
