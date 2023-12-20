#include "ContactGetter.h"
#include <fstream>

using namespace tcms;

ContactGetter::ContactGetter(id_type id) : id(id) {}

ContactGetter::~ContactGetter() {
    rc[id]--;
    if (rc[id] <= 0) {
        delete cache[id];
        rc.erase(id);
        cache.erase(id);
    }
}

fs::Path ContactGetter::get_path() const {
    return fs::Path{"metadata", std::to_string(id)};
}

id_type ContactGetter::get_id() const {
    return id;
}

Contact *ContactGetter::get() {
    try {
        return cache.at(id);
    } catch (const std::out_of_range &e) {
        // ignored
    }

    auto ba = fs::read_file(get_path());
    auto contact = Contact::deserialize(ba);
    cache[id] = contact;
    rc[id] = 1;
    return contact;
}

ContactGetter ContactGetter::from_file(const fs::Path &path) {
    std::ifstream ifs(fs::path_to_string(path));
    char *buf = (char *) malloc(sizeof(id_type));
    ifs.read(buf, sizeof(id_type));
    auto id = bytes::read_number<id_type>(buf);
    return ContactGetter(id);
}
