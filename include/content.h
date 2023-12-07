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
    };

    class BodyFrame : public Frame {
    private:
        int id;
        std::string content;
    public:
        BodyFrame(int id, std::string content);
        std::string to_string() override;
    };

    class ImageFrame : public Frame {
    private:
        int id;
        std::string caption;
    public:
        ImageFrame(int id, std::string caption);
        std::string to_string() override;
        std::string get_path();
    };

    class Article {
    private:
        int id;
    public:
        explicit Article(int id);
        ~Article();
        int get_id();
        std::vector<Frame *> get_frames();
    };
}

#endif