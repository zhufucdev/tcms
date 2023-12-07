//
// Created by Steve Reed on 2023/12/7.
//

#include "cms.h"
#include <sys/stat.h>

void tcms::initialize() {
    mkdir("content", 0777);
    mkdir("metadata", 0777);
}