#include "test.h"
#include <iostream>

test::assert_exception::assert_exception(const std::string &what_test) : runtime_error(what_test) {}

std::string std::to_string(const std::string &str) {
    return str;
}

void test::complete() {
    std::cout << "All tests completed. Nothing special happened." << std::endl;
}

void test::complete(const std::string &test) {
    std::cout << "[" << test << "] test completed" << std::endl;
}
