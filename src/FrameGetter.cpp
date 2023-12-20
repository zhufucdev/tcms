#include "FrameGetter.h"
#include <fstream>

using namespace tcms;

fs::Path get_path(id_type id) {
    return fs::Path{"tcms", "frame_getters", std::to_string(id)};
}

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

Frame *FrameGetter::get() {
    try {
        return cache.at(id);
    } catch (const std::out_of_range &e) {
        // ignored
    }

    auto ba = fs::read_file(get_path(id));
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

FrameGetter *FrameGetter::from_file(const fs::Path &path) {
    std::ifstream ifs(fs::path_to_string(path));
    char type;
    ifs >> type;
    auto buf = (char *) malloc(sizeof(id_type));
    ifs.read(buf, sizeof(id_type));
    auto id = bytes::read_number<id_type>(buf);
    ifs.close();
    return new FrameGetter(id, (FrameType) type);
}

