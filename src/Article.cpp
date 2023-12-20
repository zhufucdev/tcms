#include "Article.h"

using namespace tcms;

Article::Article(id_type id, const std::string &name, const Metadata &metadata, const std::vector<FrameGetter> &frames)
        : id(id), name(name), frames(frames), metadata(metadata) {}

Article::Article(const std::string &name) : name(name), id(increment::get_next_id()), frames(), metadata() {}

Article::~Article() = default;

int Article::get_id() const {
    return id;
}

std::vector<FrameGetter> Article::get_frames() const {
    return frames;
}

Metadata &Article::get_metadata() {
    return metadata;
}

ByteArray Article::serialize() const {
    auto mba = metadata.serialize();
    size_t len = sizeof(id_type) * (frames.size() + mba.len + 1) + sizeof(size_t);
    auto buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    size_t ptr = sizeof(id_type);
    bytes::write_number<size_t>(buf + ptr, frames.size());
    ptr += sizeof(size_t);
    for (const auto &frame: frames) {
        bytes::write_number(buf + ptr, frame.get_id());
        ptr += sizeof(id_type);
    }
    std::memcpy(buf + ptr, mba.content, mba.len);
    delete mba.content;
    return {buf, len};
}

Article *Article::deserialize(ByteArray ba) {
    size_t ptr = sizeof(id_type);
    auto id = bytes::read_number<id_type>(ba.content);
    increment::add_id(id);
    for (; ba.content[ptr] != 0 && ptr < ba.len; ptr++);
    std::string name(ba.content + sizeof(id_type), ba.content + ptr);
    ptr++;
    auto frame_count = bytes::read_number<size_t>(ba.content + ptr);
    std::vector<FrameGetter> frames;
    for (int i = 0; i < frame_count; i++) {
        auto fid = bytes::read_number<id_type>(ba.content + ptr);
        ptr += sizeof(id_type);
        frames.push_back(FrameGetter::from_file(fid));
    }
    auto metadata = Metadata::deserialize(ba - ptr);
    auto article = new Article(id, name, metadata, frames);
    return article;
}

