#include "FrameGetter.h"
#include <fstream>

using namespace tcms;

FrameGetter::FrameGetter(const id_type id, const tcms::FrameType type) : id(id), type(type) {
    increment::add_id(id);
    if (rc.find(id) == rc.end()) {
        rc[id] = 0;
    }
    rc[id]++;
}

FrameGetter::FrameGetter(const tcms::FrameGetter &f) : id(f.id) {
    rc[id]++;
}

FrameGetter::~FrameGetter() {
    rc[id]--;
    if (rc[id] <= 0) {
        rc.erase(id);
        delete cache[id];
        cache.erase(id);
    }
}

fs::Path FrameGetter::get_path() const {
    return get_path(id);
}

void FrameGetter::remove() {
    if (rc[id] <= 1) {
        fs::FileAssociated::remove();
        if (type == IMAGE) {
            auto i = dynamic_cast<ImageFrame *>(get());
            fs::remove_file(i->get_image_path());
        }
    }
}

Frame *FrameGetter::get() const {
    try {
        return cache.at(id);
    } catch (const std::out_of_range &e) {
        // ignored
    }

    auto ba = fs::read_file(get_path());
    Frame *frame;
    switch (type) {
        case FrameType::HEADER:
            frame = TitleFrame::deserialize(ba);
            break;
        case FrameType::PARAGRAPH:
            frame = ParagraphFrame::deserialize(ba);
            break;
        case FrameType::IMAGE:
            frame = ImageFrame::deserialize(ba);
            break;
        default:
            throw std::runtime_error("unknown frame type");
    }
    cache[id] = frame;
    return frame;
}

FrameGetter *FrameGetter::clone() const {
    return new FrameGetter(*this);
}

FrameType FrameGetter::get_type() const {
    return type;
}

id_type FrameGetter::get_id() const {
    return id;
}

size_t FrameGetter::estimated_size() const {
    std::ifstream ifs(fs::path_to_string(get_path(id)));
    ifs.seekg(0, std::ifstream::end);
    auto len = ifs.tellg();
    ifs.close();
    return len;
}

ByteArray FrameGetter::serialize() const {
    return get()->serialize();
}

FrameGetter *FrameGetter::from_file(id_type id) {
    std::ifstream ifs(fs::path_to_string(get_path(id)));
    char type;
    ifs.read(&type, 1);
    ifs.close();
    return new FrameGetter{id, (FrameType) type};
}

fs::Path FrameGetter::get_path(id_type id) {
    return {"frames", std::to_string(id)};
}

MemoryFrameGetter::MemoryFrameGetter(tcms::Frame *frame) :
        frame(frame), FrameGetter(frame->get_id(), frame->get_type()) {
    auto id = frame->get_id();
    cache[id] = frame;
}

Frame *MemoryFrameGetter::get() const {
    return frame;
}

FrameGetter *MemoryFrameGetter::clone() const {
    return new MemoryFrameGetter(*this);
}
