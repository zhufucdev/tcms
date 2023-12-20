#include "Article.h"

using namespace tcms;

Article::Article(id_type id) : id(id), frame_getters() {}

Article::~Article() = default;

int Article::get_id() const {
    return id;
}

std::vector<FrameGetter *> Article::get_frames() const {
    return frame_getters;
}

std::vector<Tag *> Article::get_tags() const {
    return tags;
}

ByteArray Article::serialize() const {
    size_t len = sizeof(id_type) * (frame_getters.size() + tags.size() + 1) + 2 * sizeof(size_t);
    auto buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    size_t ptr = sizeof(id_type);
    bytes::write_number<size_t>(buf + ptr, frame_getters.size());
    ptr += sizeof(size_t);
    for (auto frame_getter : frame_getters) {
        bytes::write_number(buf + ptr, frame_getter->get_id());
        ptr += sizeof(id_type);
    }
    bytes::write_number<size_t>(buf + ptr, tags.size());
    ptr += sizeof(size_t);
    for (auto tag : tags) {

    }
    return {buf, len};
}

Article *Article::deserialize(ByteArray ba) {
    size_t ptr = sizeof(id_type);
    auto id = bytes::read_number<id_type>(ba.content);
    increment::add_id(id);
    std::vector<FrameGetter *> frames((ba.len - ptr) / ptr);
    int count = 0;
    while (ptr < ba.len) {
        auto type = (FrameType) ba.content[ptr];
        auto fid = bytes::read_number<id_type>(ba.content + ptr + 1);
        ptr += sizeof(id_type);
        frames[count++] = new FrameGetter(fid, type);
    }
    auto article = new Article(id);
    article->frame_getters = frames;
    return article;
}

