#include "ContactGetter.h"

using namespace tcms;

std::map<id_type, size_t> ContactGetter::rc{};
std::map<id_type, Contact *> ContactGetter::cache{};

ContactGetter::ContactGetter(id_type id) : id(id) {}

ContactGetter::~ContactGetter() {
    rc[id]--;
    if (rc[id] <= 0) {
        delete cache[id];
        rc.erase(id);
        cache.erase(id);
    }
}

fs::Path get_contact_path(id_type id) {
    return fs::Path{"metadata", std::to_string(id)};
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

    auto ba = fs::read_file(get_contact_path(id));
    auto contact = Contact::deserialize(ba);
    cache[id] = contact;
    rc[id] = 1;
    return contact;
}

MemoryContactGetter::MemoryContactGetter(tcms::Contact *contact)
        : contact(contact), ContactGetter(contact->get_id()) {
    cache[contact->get_id()] = contact;
    rc[contact->get_id()] = 1;
}

Contact *MemoryContactGetter::get() const {
    return contact;
}
