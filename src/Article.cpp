#include "Article.h"

using namespace tcms;

Article::Article(id_type id) : id(id), frame_getters() {}

Article::~Article() = default;

int Article::get_id() const {
    return id;
}

std::vector<FrameGetter *> Article::get_frames() {
    return frame_getters;
}

ByteArray Article::serialize() const {
    size_t len = sizeof(id_type) * (1 + frame_getters.size());
    auto buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    for (int i = 0; i < frame_getters.size(); ++i) {
        bytes::write_number(buf + (i + 1) * sizeof(char), frame_getters[i]->get_id());
    }
    return {buf, len};
}

Article *Article::deserialize(ByteArray ba) {
    size_t ptr = sizeof(id_type);
    auto id = bytes::read_number<id_type>(ba.content);
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

