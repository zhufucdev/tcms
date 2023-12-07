//
// Created by Steve Reed on 2023/12/7.
//

#include "cms.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void tcms::initialize() {
    mkdir("content", 0777);
    mkdir("metadata", 0777);
}