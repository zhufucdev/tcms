//
// Created by Steve Reed on 2023/12/19.
//

#ifndef TCMS_FRAMEGETTER_H
#define TCMS_FRAMEGETTER_H

#include "increment.h"
#include "content.h"

namespace tcms {
    class FrameGetter {
        id_type id;
        FrameType type;
        Frame *cache;

    public:
        FrameGetter(id_type id, FrameType type);

        ~FrameGetter();

        Frame *get();

        FrameType get_type() const;

        id_type get_id() const;

        static FrameGetter *from_file(const fs::Path &path);
    };
}


#endif //TCMS_FRAMEGETTER_H
