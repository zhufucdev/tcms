#ifndef TCMS_CONTACTGETTER_H
#define TCMS_CONTACTGETTER_H

#include "increment.h"
#include "Contact.h"
#include "fs.h"
#include <map>


namespace tcms {
    class ContactGetter : public fs::FileAssociated {
        id_type id;

    protected:
        static std::map<id_type, Contact *> cache;
        static std::map<id_type, size_t> rc;

    public:
        explicit ContactGetter(id_type id);

        ContactGetter(const ContactGetter &c);

        ~ContactGetter();

        fs::Path get_path() const override;

        id_type get_id() const;

        virtual Contact *get() const;

        virtual ContactGetter *clone() const;

        ByteArray serialize() const override;

        static fs::Path get_path(id_type id);
    };

    class MemoryContactGetter : public ContactGetter {
        Contact *contact;
    public:
        explicit MemoryContactGetter(Contact *contact);

        Contact *get() const override;

        ContactGetter * clone() const override;
    };
}


#endif //TCMS_CONTACTGETTER_H
