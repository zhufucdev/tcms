#ifndef TCMS_CONTENT_H
#define TCMS_CONTENT_H

#include <vector>
#include <string>
#include "bytes.h"
#include "increment.h"

namespace tcms {
    class Frame : public bytes::BinarySerializable {
        id_type id;
    public:
        Frame(int id);
        int get_id();
        virtual std::string to_string() = 0;
    };

    class TitleFrame : public Frame {
    private:
        int id, depth;
        std::string content;
    public:
        TitleFrame(int id, std::string content, int depth);
        std::string to_string() override;
        ByteArray serialize() const override;
        static TitleFrame deserialize(ByteArray ba);
    };

    class ParagraphFrame : public Frame {
    private:
        int id;
        std::string content;
    public:
        ParagraphFrame(int id, std::string content);
        std::string to_string() override;
        ByteArray serialize() const override;
        static ParagraphFrame deserialize(ByteArray ba);
    };

    class ImageFrame : public Frame {
    private:
        int id;
        std::string caption;
    public:
        ImageFrame(int id, std::string caption);
        std::string to_string() override;
        std::string get_path();
        ByteArray serialize() const override;
        static ImageFrame deserialize(ByteArray ba);
    };

    class Article : public bytes::BinarySerializable {
    private:
        int id;
    public:
        explicit Article(int id);
        ~Article();
        int get_id();
        std::vector<Frame *> get_frames();
        ByteArray serialize() const override;
        static Article deserialize(ByteArray ba);
    };
}

#endif