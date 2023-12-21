#include "ContactGetter.h"
#include <stdexcept>

using namespace tcms;

ContactGetter::ContactGetter(id_type id) : id(id) {
    if (rc.find(id) == rc.end()) {
        rc[id] = 0;
    }
    rc[id]++;
}

ContactGetter::~ContactGetter() {
    rc[id]--;
    if (rc[id] <= 0) {
        delete cache[id];
        rc.erase(id);
        cache.erase(id);
    }
}

fs::Path ContactGetter::get_path() const {
    return get_path(id);
}

id_type ContactGetter::get_id() const {
    return id;
}

Contact *ContactGetter::get() const {
    try {
        return cache.at(id);
    } catch (const std::out_of_range &e) {
        // ignored
    }

    auto ba = fs::read_file(get_path(id));
    auto contact = Contact::deserialize(ba);
    cache[id] = contact;
    return contact;
}

ByteArray ContactGetter::serialize() const {
    return get()->serialize();
}

fs::Path ContactGetter::get_path(id_type id) {
    return {"contacts", std::to_string(id)};
}

MemoryContactGetter::MemoryContactGetter(tcms::Contact *contact)
        : contact(contact), ContactGetter(contact->get_id()) {
    auto id = contact->get_id();
    cache[id] = contact;
}

Contact *MemoryContactGetter::get() const {
    return contact;
}
