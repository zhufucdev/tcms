//
// Created by Steve Reed on 2023/12/19.
//

#ifndef TCMS_FRAMEGETTER_H
#define TCMS_FRAMEGETTER_H

#include "increment.h"
#include "content.h"
#include <map>

namespace tcms {
    class FrameGetter {
        id_type id;
        FrameType type;

    protected:
        static std::map<id_type, Frame *> cache;
        static std::map<id_type, size_t> rc;

    public:
        FrameGetter(id_type id, FrameType type);

        ~FrameGetter();

        virtual Frame *get() const;

        FrameType get_type() const;

        id_type get_id() const;

        static FrameGetter *from_file(id_type id);
    };

    class MemoryFrameGetter : public FrameGetter {
        Frame *frame;
    public:
        explicit MemoryFrameGetter(Frame *frame);

        Frame *get() const override;
    };
}


#endif //TCMS_FRAMEGETTER_H
