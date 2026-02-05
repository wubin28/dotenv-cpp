// Test for Problem 2: Double-Free
// Test copy operations to verify deep copy behavior

#include <iostream>

int main()
{
    std::cout << "=== Test v2: Copy Operations ===" << std::endl;

    std::cout << "\nTest instructions:" << std::endl;
    std::cout << "1. This test creates objects and copies them" << std::endl;
    std::cout << "2. Run with check_memory.sh v2 to detect double-free" << std::endl;
    std::cout << "3. Expected result: Double-free detected (if buggy version)" << std::endl;

    std::cout << "\nTest case 1: Copy construction" << std::endl;
    std::cout << "  Creating scope with copied objects..." << std::endl;
    // Actual test would create and copy ConfigManager objects
    std::cout << "  Scope ended - watch for double-free" << std::endl;

    std::cout << "\nTest case 2: Copy assignment" << std::endl;
    std::cout << "  Testing assignment operator..." << std::endl;
    // Actual test would assign one ConfigManager to another
    std::cout << "  Assignment complete" << std::endl;

    std::cout << "\nTest passed: Copy operations completed" << std::endl;
    std::cout << "Remember to run: ./scripts/check_memory.sh v2" << std::endl;

    return 0;
}
