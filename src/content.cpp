#include "content.h"
#include "fs.h"
#include <stdexcept>

using namespace tcms;

Article::Article(id_type id) : id(id), frames() {}

Article::~Article() = default;

int Article::get_id() const {
    return id;
}

std::vector<Frame *> Article::get_frames() {
    return frames;
}

Frame::Frame(int id) : id(id) {}

id_type Frame::get_id() const {
    return id;
}

ByteArray Article::serialize() const {
    size_t len = sizeof(id_type) * (1 + frames.size());
    auto buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    for (int i = 0; i < frames.size(); ++i) {
        bytes::write_number(buf + (i + 1) * sizeof(char), frames[i]->get_id());
    }
    return {buf, len};
}

Article Article::deserialize(ByteArray ba, const std::function<Frame *(id_type)> &getter) {
    size_t ptr = sizeof(id_type);
    auto id = bytes::read_number<id_type>(ba.content);
    std::vector<Frame *> frames((ba.len - ptr) / ptr);
    int count = 0;
    while (ptr < ba.len) {
        auto fid = bytes::read_number<id_type>(ba.content + ptr);
        ptr += sizeof(id_type);
        frames[count++] = getter(fid);
    }
    auto article = Article(id);
    article.frames = frames;
    return article;
}

TitleFrame::TitleFrame(id_type id, const std::string &content, int depth) : Frame(id), depth(depth), content(content) {}

std::string TitleFrame::to_string() {
    return content;
}

ByteArray TitleFrame::serialize() const {
    size_t len = sizeof(id_type) + sizeof(depth) + content.length() + 1;
    char *buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, len);
    bytes::write_number(buf + sizeof(id_type), depth);
    std::memcpy(buf + sizeof(id_type) + sizeof(char), content.c_str(), content.length());
    return {buf, len};
}

TitleFrame TitleFrame::deserialize(ByteArray ba) {
    auto id = bytes::read_number<id_type>(ba.content);
    auto depth = bytes::read_number<int>(ba.content + sizeof(id_type));
    auto content = (ba + sizeof(id_type) + sizeof(int)).content;
    return {id, content, depth};
}

ParagraphFrame::ParagraphFrame(id_type id, const std::string &content) : Frame(id), content(content) {}

std::string ParagraphFrame::to_string() {
    return content;
}

ByteArray ParagraphFrame::serialize() const {
    size_t len = sizeof(id_type) + content.length() + 1;
    char *buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    std::memcpy(buf + sizeof(char), content.c_str(), content.length());
    return {buf, len};
}

ParagraphFrame ParagraphFrame::deserialize(ByteArray ba) {
    auto id = bytes::read_number<id_type>(ba.content);
    auto content = (ba + sizeof(id_type)).content;
    return {id, content};
}

ImageFrame::ImageFrame(id_type id, const std::string &caption)
        : Frame(id), caption(caption), extension() {}

std::string ImageFrame::to_string() {
    return caption;
}

std::string ImageFrame::get_path() {
    if (extension.empty()) {
        return "";
    }
    return std::to_string(id) + "." + extension;
}

void ImageFrame::set_file(const std::string &path) {
    if (fs::copy(path, get_path())) {
        extension = fs::get_extension(path);
    } else {
        throw std::runtime_error("error while copying image file (copying " + path + ")");
    }
}

ByteArray ImageFrame::serialize() const {
    auto sba = bytes::from_string(caption, extension);
    size_t len = sizeof(id_type) + sba.len;
    char *buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    std::memcpy(buf + sizeof(char), sba.content, sba.len);
    delete sba.content;
    return {buf, len};
}

ImageFrame ImageFrame::deserialize(ByteArray ba) {
    auto id = bytes::read_number<id_type>(ba.content);
    auto sa = bytes::to_string(ba + sizeof(id_type));
    if (sa.size() != 2) {
        throw std::runtime_error("invalid data (deserializing ImageFrame)");
    }
    ImageFrame frame{id, sa[0]};
    frame.extension = sa[1];
    return frame;
}
