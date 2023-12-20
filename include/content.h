#ifndef TCMS_CONTENT_H
#define TCMS_CONTENT_H

#include <vector>
#include <string>
#include <functional>
#include "bytes.h"
#include "increment.h"
#include "fs.h"

namespace tcms {
    enum FrameType {
        TITLE = 0,
        PARAGRAPH,
        IMAGE
    };

    class Frame : public bytes::BinarySerializable {
        id_type id;
    public:
        explicit Frame(id_type id);

        id_type get_id() const;

        virtual FrameType get_type() const = 0;

        virtual std::string to_string() = 0;
    };


    class TitleFrame : public Frame {
    private:
        id_type id;
        int depth;
        std::string content;

        TitleFrame(id_type id, const std::string &content, int depth);

    public:
        TitleFrame(const std::string &content, int depth);

        FrameType get_type() const override;

        std::string to_string() override;

        ByteArray serialize() const override;

        static TitleFrame *deserialize(ByteArray ba);
    };

    class ParagraphFrame : public Frame {
    private:
        id_type id;
        std::string content;

        ParagraphFrame(id_type id, const std::string &content);

    public:
        ParagraphFrame(const std::string &content);

        FrameType get_type() const override;

        std::string to_string() override;

        ByteArray serialize() const override;

        static ParagraphFrame *deserialize(ByteArray ba);
    };

    class ImageFrame : public Frame {
    private:
        id_type id;
        std::string caption, extension;

        ImageFrame(id_type id, const std::string &caption);

    public:
        explicit ImageFrame(const std::string &caption);

        FrameType get_type() const override;

        std::string to_string() override;

        void set_file(const fs::Path &path);

        fs::Path get_path();

        ByteArray serialize() const override;

        static ImageFrame *deserialize(ByteArray ba);
    };

}

#endif