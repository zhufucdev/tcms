#ifndef TCMS_CONTACTGETTER_H
#define TCMS_CONTACTGETTER_H

#include "increment.h"
#include "Contact.h"
#include "fs.h"
#include <map>


namespace tcms {
    class ContactGetter {
        id_type id;

    protected:
        static std::map<id_type, Contact *> cache;
        static std::map<id_type, size_t> rc;

    public:
        explicit ContactGetter(id_type id);

        ~ContactGetter();

        id_type get_id() const;

        virtual Contact *get() const;
    };

    class ConstantContactGetter : public ContactGetter {
        Contact *contact;
    public:
        explicit ConstantContactGetter(Contact *contact);

        Contact * get() const override;
    };
}


#endif //TCMS_CONTACTGETTER_H
