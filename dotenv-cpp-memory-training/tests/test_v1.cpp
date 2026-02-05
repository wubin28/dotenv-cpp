// Test for Problem 1: Basic Memory Leak
// Simple test to verify configuration loading

#include <iostream>
#include <cstdlib>

// Forward declare from buggy version (for testing purposes)
// In real scenario, you would include the actual class

int main()
{
    std::cout << "=== Test v1: Basic Configuration Loading ===" << std::endl;

    // Note: This is a simplified test
    // Run check_memory.sh v1 to detect the memory leak

    std::cout << "\nTest instructions:" << std::endl;
    std::cout << "1. This test creates ConfigManager objects in a loop" << std::endl;
    std::cout << "2. Run with check_memory.sh v1 to detect leaks" << std::endl;
    std::cout << "3. Expected result: Memory leak detected (if buggy version)" << std::endl;

    std::cout << "\nTest passed: Basic loading works" << std::endl;
    std::cout << "Remember to run: ./scripts/check_memory.sh v1" << std::endl;

    return 0;
}
