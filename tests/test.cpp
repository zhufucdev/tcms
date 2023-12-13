#include "test.h"
#include <iostream>

test::assert_exception::assert_exception(const std::string &what_test, const std::string &what_failed)
        : test_name(what_test), message(what_failed) {}

const char *test::assert_exception::what() const noexcept {
    auto buf = (char *) malloc(message.size());
    std::memcpy(buf, message.c_str(), message.size());
    return buf;
}

const std::string &test::assert_exception::what_test() const noexcept {
    return test_name;
}

std::string std::to_string(const std::string &str) {
    return str;
}

void test::completed() {
    std::cout << "All tests completed. Nothing special happened." << std::endl;
}

void test::completed(const std::string &test) {
    std::cout << "[" << test << "] test completed" << std::endl;
}

void test::failed(const std::vector<std::string> &failed_tests) {
    std::cout << "Test completed with following failures: \n\t";
    int i = 0;
    for (; i < failed_tests.size() - 1; i++) {
        std::cout << failed_tests[i] << ", ";
        if (i % 5 == 0) {
            std::cout << "\n\t";
        }
    }
    std::cout << failed_tests[i] << std::endl;
}

void test::run_tests(const std::vector<void (*)()> &tests) {
    auto failures = std::vector<std::string>();
    for (auto test: tests) {
        try {
            test();
        } catch (const assert_exception &e) {
            std::cout << "[" << e.what_test() << "] " << e.what() << std::endl;
            failures.push_back(e.what_test());
        }
    }
    if (failures.empty()) {
        completed();
    } else {
        failed(failures);
    }
}
