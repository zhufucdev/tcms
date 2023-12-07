#include <string>
#include <vector>
#include "bytes.h"

namespace tcms {
    class Contact {
        int id;
        std::vector<std::string> names;
        Contact(const std::vector<std::string> &names);
    public:
        Contact(std::string first_name);
        const std::vector<std::string> &get_names() const;
        std::string get_full_name() const;
        ByteArray serialize();
        Contact *deserialize(ByteArray ba);
    };
}