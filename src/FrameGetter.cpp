#include "FrameGetter.h"
#include <fstream>

using namespace tcms;

std::map<id_type, Frame *> FrameGetter::cache{};
std::map<id_type, size_t> FrameGetter::rc{};

FrameGetter::FrameGetter(const id_type id, const tcms::FrameType type) : id(id), type(type) {}

FrameGetter::~FrameGetter() {
    rc[id]--;
    if (rc[id] <= 0) {
        rc.erase(id);
        delete cache[id];
        cache.erase(id);
    }
}

Frame *FrameGetter::get() const {
    try {
        return cache.at(id);
    } catch (const std::out_of_range &e) {
        // ignored
    }

    auto ba = fs::read_file(Frame::get_path(id));
    Frame *frame;
    switch (type) {
        case FrameType::TITLE:
            frame = TitleFrame::deserialize(ba);
        case FrameType::PARAGRAPH:
            frame = ParagraphFrame::deserialize(ba);
        case FrameType::IMAGE:
            frame = ImageFrame::deserialize(ba);
        default:
            throw std::runtime_error("unknown frame type");
    }
    cache[id] = frame;
    rc[id] = 1;
    return frame;
}

FrameType FrameGetter::get_type() const {
    return type;
}

id_type FrameGetter::get_id() const {
    return id;
}

FrameGetter *FrameGetter::from_file(id_type id) {
    std::ifstream ifs(fs::path_to_string(Frame::get_path(id)));
    char type;
    ifs >> type;
    ifs.close();
    return new FrameGetter{id, (FrameType) type};
}

MemoryFrameGetter::MemoryFrameGetter(tcms::Frame *frame) :
        frame(frame), FrameGetter(frame->get_id(), frame->get_type()) {
    auto id = frame->get_id();
    cache[id] = frame;
    if (rc.count(id)) {
        rc[id]++;
    } else {
        rc[id] = 1;
    }
}

Frame *MemoryFrameGetter::get() const {
    return frame;
}
