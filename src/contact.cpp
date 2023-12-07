#include "contact.h"
#include "increment.h"

tcms::Contact::Contact(const std::string &first_name) : names{first_name}, id(increment::get_next_id()) {}

tcms::Contact::Contact(id_type id, const std::vector<std::string> &names) : id(id), names(names) {}

const std::vector<std::string> &tcms::Contact::get_names() const {
    return names;
}

std::string tcms::Contact::get_full_name() const {
    std::string full_name;
    int i;
    for (i = 0; i < names.size() - 1; i++) {
        full_name += names[i] + " ";
    }
    for (; i < names.size(); i++) {
        full_name += names[i];
    }
    return full_name;
}

ByteArray tcms::Contact::serialize() const {
    size_t len = sizeof(id);
    for (const auto &n: names) {
        len += n.length() + 1;
    }
    char *buf = (char *) calloc(len, sizeof(char));
    bytes::write_number(buf, id);
    len = sizeof(id);
    for (const auto &n: names) {
        std::memcpy(buf + len, n.c_str(), n.length());
        len += n.length() + 1;
    }
    return {buf, len};
}

tcms::Contact *tcms::Contact::deserialize(ByteArray ba) {
    auto id = bytes::read_number<id_type>(ba.content);
    auto names = bytes::to_string(ba + sizeof(id_type));
    return new Contact{id, names};
}

void tcms::Contact::set_name(size_t index, const std::string &name) {
    if (names.size() < index + 1) {
        names.push_back(name);
    } else {
        names[index] = name;
    }
}
