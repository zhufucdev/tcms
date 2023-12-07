#ifndef TCMS_INCREMENT_H
#define TCMS_INCREMENT_H

typedef int id_type;

namespace increment {
    void add_id(id_type curr);
    id_type get_next_id();
}

#endif