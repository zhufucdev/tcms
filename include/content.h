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

        virtual std::string to_string() = 0;
    };


    class TitleFrame : public Frame {
    private:
        id_type id;
        int depth;
        std::string content;
    public:
        TitleFrame(id_type id, const std::string &content, int depth);

        std::string to_string() override;

        ByteArray serialize() const override;

        static TitleFrame *deserialize(ByteArray ba);
    };

    class ParagraphFrame : public Frame {
    private:
        id_type id;
        std::string content;
    public:
        ParagraphFrame(id_type id, const std::string &content);

        std::string to_string() override;

        ByteArray serialize() const override;

        static ParagraphFrame *deserialize(ByteArray ba);
    };

    class ImageFrame : public Frame {
    private:
        id_type id;
        std::string caption, extension;
    public:
        ImageFrame(id_type id, const std::string &caption);

        std::string to_string() override;

        void set_file(const fs::Path &path);

        fs::Path get_path();

        ByteArray serialize() const override;

        static ImageFrame *deserialize(ByteArray ba);
    };

}

#endif