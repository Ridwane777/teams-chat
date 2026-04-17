#include "test_helpers.hpp"

int main()
{
    int failed = 0;
    for (const auto &test : test_registry()) {
        try {
            test.fn();
            std::cout << "[OK] " << test.name << "\n";
        } catch (const std::exception &e) {
            ++failed;
            std::cerr << "[KO] " << test.name << ": " << e.what() << "\n";
        }
    }
    std::cout << "Executed " << test_registry().size() << " tests\n";
    return failed == 0 ? 0 : 1;
}
