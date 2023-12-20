#ifndef TCMS_CONTACT_H
#define TCMS_CONTACT_H

#include <string>
#include <vector>
#include "fs.h"
#include "increment.h"

namespace tcms {
    class Contact : public fs::FileWritable {
        id_type id;
        std::vector<std::string> names;

        Contact(id_type id, const std::vector<std::string> &names);

    public:
        explicit Contact(const std::string &first_name);

        id_type get_id() const;

        fs::Path get_path() const override;

        void set_name(size_t index, const std::string &name);

        const std::vector<std::string> &get_names() const;

        std::string get_full_name() const;

        ByteArray serialize() const override;

        static Contact *deserialize(ByteArray ba);

        static fs::Path get_path(id_type id);
    };
}

#endif