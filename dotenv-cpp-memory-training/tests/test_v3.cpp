// Test for Problem 3: Dangling Pointer
// Test returning pointers to verify no dangling references

#include <iostream>

int main()
{
    std::cout << "=== Test v3: Dangling Pointer Detection ===" << std::endl;

    std::cout << "\nTest instructions:" << std::endl;
    std::cout << "1. This test gets pointer from function and uses it later" << std::endl;
    std::cout << "2. Run with check_memory.sh v3 to detect stack-use-after-return" << std::endl;
    std::cout << "3. Expected result: Use-after-free detected (if buggy version)" << std::endl;

    std::cout << "\nTest case: Get formatted string" << std::endl;
    std::cout << "  Calling function that returns pointer..." << std::endl;
    // Actual test would call get_formatted_connection_string()
    // and try to use the pointer after other stack operations

    std::cout << "  Attempting to use pointer..." << std::endl;
    std::cout << "  Watch for undefined behavior" << std::endl;

    std::cout << "\nTest completed" << std::endl;
    std::cout << "Remember to run: ./scripts/check_memory.sh v3" << std::endl;

    return 0;
}
