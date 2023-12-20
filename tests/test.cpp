#include "test.h"
#include "terminal.h"
#include <iostream>

static int all_test_count = 0;
static int current_test = 0;

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
    terminal::clear_screen();
    std::cout << "[" << test << "] (" << current_test + 1 << "/" << all_test_count << ") test completed" << std::endl;
}

void test::failed(const std::vector<std::string> &failed_tests) {
    std::cerr << "Test completed with following failures: \n\t";
    int i = 0;
    for (; i < failed_tests.size() - 1; i++) {
        std::cerr << failed_tests[i] << ", ";
        if (i % 5 == 0) {
            std::cerr << "\n\t";
        }
    }
    std::cerr << failed_tests[i] << std::endl;
}

void test::run_tests(const std::vector<void (*)()> &tests) {
    all_test_count = tests.size();
    auto failures = std::vector<std::string>();
    for (current_test = 0; current_test < tests.size(); current_test++) {
        try {
            tests[current_test]();
        } catch (const assert_exception &e) {
            std::cout << "[" << e.what_test() << "] " << e.what() << std::endl;
            failures.push_back(e.what_test());
        } catch (const std::exception &e) {
            std::cerr << "A unit failed for unknown error: " << e.what() << std::endl;
            failures.emplace_back("known");
        }
    }
    if (failures.empty()) {
        completed();
    } else {
        failed(failures);
    }
}
