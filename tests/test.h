#include <stdexcept>
#include <string>
#include <vector>

namespace std {
    std::string to_string(const std::string &str);

    template<typename T>
    std::string to_string(const std::vector<T> &v) {
        int i;
        std::string r = "[";
        for (i = 0; i < v.size() - 1; i++) {
            r += std::to_string(v[i]) + ", ";
        }
        for (; i < v.size(); i++) {
            r += std::to_string(v[i]) + "]";
        }
        return r;
    }
}

namespace test {
    void completed();

    void completed(const std::string &test);

    void failed(const std::vector<std::string> &failed_tests);

    class assert_exception : public std::exception {
        const std::string test_name, message;
    public:
        explicit assert_exception(const std::string &what_test, const std::string &what_failed);

        const char *what() const noexcept override;

        const std::string &what_test() const noexcept;
    };

    template<class T1, class T2>
    class assert_eq_exception : public assert_exception {
    public:
        assert_eq_exception(const std::string &what_test, T1 expected, T2 actual) : assert_exception(
                what_test,
                "assert failed\n\tExpected: " + std::to_string(expected) + "\n\tActual: " + std::to_string(actual)) {};
    };

    template<class T1, class T2>
    inline void assert_eq(const std::string &test, T1 expected, T2 actual) {
        if (expected != actual) {
            throw assert_eq_exception<T1, T2>(test, expected, actual);
        }
        completed(test);
    }

    inline void assert(const std::string &test, bool assumption) {
        if (!assumption) {
            throw assert_exception(test, "assert failed");
        }
        completed(test);
    }

    void run_tests(const std::vector<void (*)()> &tests);
}