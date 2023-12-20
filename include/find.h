//
// Created by Steve Reed on 2023/12/20.
//

#ifndef TCMS_FIND_H
#define TCMS_FIND_H

namespace find {
    template<typename T, class Iterator>
    T by_id(Iterator first, Iterator last, id_type id) {
        while (first != last) {
            if ((*first)->get_id() == id) {
                return *first;
            }
            first++;
        }
        return nullptr;
    }
}

#endif //TCMS_FIND_H
