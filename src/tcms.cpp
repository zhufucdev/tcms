#include "tcms.h"
#include "fs.h"

void tcms::TCMS::initialize() {
    fs::create_directory("content");
    fs::create_directory("metadata");
    fs::create_directory("frames");
    fs::create_directory("metadata/tag");
}
