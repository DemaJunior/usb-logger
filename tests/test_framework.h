#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

void register_test(const std::string& name, std::function<void()> fn);

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> fn) {
        register_test(name, std::move(fn));
    }
};

#define TEST_CONCAT_INNER(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT_INNER(a, b)

#define TEST_CASE(name) \
    static void TEST_CONCAT(test_fn_, __LINE__)(); \
    static TestRegistrar TEST_CONCAT(test_reg_, __LINE__){name, TEST_CONCAT(test_fn_, __LINE__)}; \
    static void TEST_CONCAT(test_fn_, __LINE__)()

inline void test_require(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": REQUIRE failed: " + expr);
    }
}

#define REQUIRE(expr) test_require((expr), #expr, __FILE__, __LINE__)
