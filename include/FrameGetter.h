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
        static std::map<id_type, Frame *> cache;
        static std::map<id_type, size_t> rc;

    public:
        FrameGetter(id_type id, FrameType type);

        ~FrameGetter();

        Frame *get();

        FrameType get_type() const;

        id_type get_id() const;

        static FrameGetter from_file(const fs::Path &path);
    };
}


#endif //TCMS_FRAMEGETTER_H
