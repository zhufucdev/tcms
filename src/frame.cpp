#include "frame.h"
#include "fs.h"
#include <stdexcept>

using namespace tcms;

Frame::Frame(int id) : id(id) {}

id_type Frame::get_id() const {
    return id;
}

fs::Path Frame::get_path() const {
    return get_path(id);
}

fs::Path Frame::get_path(id_type id) {
    return {"frames", std::to_string(id)};
}

TitleFrame::TitleFrame(id_type id, const std::string &content, int depth) : Frame(id), depth(depth), content(content) {}

TitleFrame::TitleFrame(const std::string &content, int depth)
        : Frame(increment::get_next_id()), depth(depth), content(content) {}

FrameType TitleFrame::get_type() const {
    return FrameType::PARAGRAPH;
}

std::string TitleFrame::to_string() {
    return content;
}

ByteArray TitleFrame::serialize() const {
    size_t len = sizeof(id_type) + sizeof(depth) + content.length() + 2;
    char *buf = (char *) calloc(len, sizeof(char));
    buf[0] = FrameType::TITLE;
    bytes::write_number(buf + 1, id);
    bytes::write_number(buf + 1 + sizeof(id_type), depth);
    std::memcpy(buf + 1 + sizeof(id_type) + sizeof(depth), content.c_str(), content.length());
    return {buf, len};
}

TitleFrame *TitleFrame::deserialize(ByteArray ba) {
    if (ba.content[0] != FrameType::TITLE) {
        throw std::runtime_error("unexpected header (deserializing TitleFrame)");
    }
    auto id = bytes::read_number<id_type>(ba.content + 1);
    auto depth = bytes::read_number<unsigned char>(ba.content + 1 + sizeof(id_type));
    auto content = (ba + 1 + sizeof(id_type) + sizeof(unsigned char)).content;
    return new TitleFrame{id, content, depth};
}

ParagraphFrame::ParagraphFrame(id_type id, const std::string &content) : Frame(id), content(content) {}

ParagraphFrame::ParagraphFrame(const std::string &content) : Frame(increment::get_next_id()), content(content) {}

FrameType ParagraphFrame::get_type() const {
    return FrameType::PARAGRAPH;
}

std::string ParagraphFrame::to_string() {
    return content;
}

ByteArray ParagraphFrame::serialize() const {
    size_t len = sizeof(id_type) + content.length() + 2;
    char *buf = (char *) calloc(len, sizeof(char));
    buf[0] = FrameType::PARAGRAPH;
    bytes::write_number(buf + 1, id);
    std::memcpy(buf + 1 + sizeof(char), content.c_str(), content.length());
    return {buf, len};
}

ParagraphFrame *ParagraphFrame::deserialize(ByteArray ba) {
    if (ba.content[0]) {
        throw std::runtime_error("unexpected header (deserializing ParagraphFrame)");
    }
    auto id = bytes::read_number<id_type>(ba.content + 1);
    auto content = (ba + 1 + sizeof(id_type)).content;
    return new ParagraphFrame{id, content};
}

ImageFrame::ImageFrame(id_type id, const std::string &caption)
        : Frame(id), caption(caption), extension() {}

ImageFrame::ImageFrame(const std::string &caption) : caption(caption), Frame(increment::get_next_id()) {}

FrameType ImageFrame::get_type() const {
    return FrameType::IMAGE;
}

std::string ImageFrame::to_string() {
    return caption;
}

fs::Path ImageFrame::get_image_path() {
    if (extension.empty()) {
        return {};
    }
    return fs::Path{"images", std::to_string(id) + extension};
}

void ImageFrame::set_file(const fs::Path &path) {
    extension = fs::get_extension(path);
    if (!fs::copy(path, get_image_path())) {
        extension = "";
        throw std::runtime_error("error while copying image file (copying " + fs::path_to_string(path) + ")");
    }
}

ByteArray ImageFrame::serialize() const {
    auto sba = bytes::from_string(caption, extension);
    size_t len = sizeof(id_type) + 1 + sba.len;
    char *buf = (char *) calloc(len, sizeof(char));
    buf[0] = FrameType::IMAGE;
    bytes::write_number(buf + 1, id);
    std::memcpy(buf + 1 + sizeof(char), sba.content, sba.len);
    delete sba.content;
    return {buf, len};
}

ImageFrame *ImageFrame::deserialize(ByteArray ba) {
    if (ba.content[0] != FrameType::IMAGE) {
        throw std::runtime_error("unexpected header (deserializing ImageFrame)");
    }
    auto id = bytes::read_number<id_type>(ba.content + 1);
    auto sa = bytes::to_string(ba + 1 + sizeof(id_type));
    if (sa.size() != 2) {
        throw std::runtime_error("invalid data (deserializing ImageFrame)");
    }
    auto *frame = new ImageFrame{id, sa[0]};
    frame->extension = sa[1];
    return frame;
}

