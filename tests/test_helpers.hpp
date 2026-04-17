#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

struct TestCase {
    const char *name;
    std::function<void()> fn;
};

inline std::vector<TestCase> &test_registry()
{
    static std::vector<TestCase> tests;
    return tests;
}

inline void register_test(const char *name, std::function<void()> fn)
{
    test_registry().push_back({name, std::move(fn)});
}

#define TEST_CASE(name) \
    static void name(); \
    static struct name##_registrar { \
        name##_registrar() { register_test(#name, name); } \
    } name##_registrar_instance; \
    static void name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            std::ostringstream _oss; \
            _oss << "ASSERT_TRUE failed: " #cond; \
            throw std::runtime_error(_oss.str()); \
        } \
    } while (0)

#define ASSERT_EQ(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a == _b)) { \
            std::ostringstream _oss; \
            _oss << "ASSERT_EQ failed: " #a " != " #b; \
            throw std::runtime_error(_oss.str()); \
        } \
    } while (0)

inline std::string read_all_from_fd(int fd)
{
    std::string out;
    char buffer[4096];
    ssize_t n = 0;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0)
        out.append(buffer, static_cast<size_t>(n));
    return out;
}

#endif
