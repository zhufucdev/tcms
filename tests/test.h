#include <exception>
#include <string>

namespace std {
    std::string to_string(const std::string& str);
}

namespace test {
    void complete();
    void complete(const std::string &test);

    class assert_exception : public std::runtime_error {
    public:
        explicit assert_exception(const std::string &what_test);
    };

    template<class T1, class T2>
    class assert_eq_exception : assert_exception {
    public:
        assert_eq_exception(const std::string &what_test, T1 expected, T2 actual) : assert_exception(
                what_test + ": assert failed.\n\tExpected: " + std::to_string(expected) + "\n\tActual: " + std::to_string(actual)) {};
    };

    template<class T1, class T2>
    inline void assert_eq(const std::string &test, T1 expected, T2 actual) {
        if (expected != actual) {
            throw assert_eq_exception<T1, T2>(test, expected, actual);
        }
        complete(test);
    }

    inline void assert(const std::string &test, bool assumption) {
        if (!assumption) {
            throw assert_exception(test);
        }
        complete(test);
    }
}