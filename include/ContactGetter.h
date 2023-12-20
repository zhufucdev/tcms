#ifndef TCMS_CONTACTGETTER_H
#define TCMS_CONTACTGETTER_H

#include "increment.h"
#include "Contact.h"
#include "fs.h"
#include <map>


namespace tcms {
    class ContactGetter {
        id_type id;
        static std::map<id_type, Contact *> cache;
        static std::map<id_type, size_t> rc;

        fs::Path get_path() const;

    public:
        explicit ContactGetter(id_type id);

        ~ContactGetter();

        id_type get_id() const;

        Contact *get();

        static ContactGetter from_file(const fs::Path &path);
    };
}


#endif //TCMS_CONTACTGETTER_H
