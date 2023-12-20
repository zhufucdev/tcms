#include "increment.h"

static id_type next_id = 0;

void increment::add_id(id_type curr) {
    if (next_id <= curr) {
        next_id = curr + 1;
    }
}

id_type increment::get_next_id() {
    return next_id++;
}
