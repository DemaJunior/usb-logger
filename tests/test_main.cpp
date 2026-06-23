#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

static std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

void register_test(const std::string& name, std::function<void()> fn) {
    registry().push_back(TestCase{name, std::move(fn)});
}

static int run_all() {
    int failed = 0;
    for (const auto& tc : registry()) {
        try {
            tc.fn();
            std::cout << "[PASS] " << tc.name << "\n";
        } catch (const std::exception& ex) {
            ++failed;
            std::cout << "[FAIL] " << tc.name << ": " << ex.what() << "\n";
        } catch (...) {
            ++failed;
            std::cout << "[FAIL] " << tc.name << ": unknown exception\n";
        }
    }
    std::cout << "\nTotal: " << registry().size() << ", Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

int main() {
    return run_all();
}
