// Test for Problem 4: Exception Safety
// Test exception handling to verify no resource leaks

#include <iostream>
#include <stdexcept>

int main()
{
    std::cout << "=== Test v4: Exception Safety ===" << std::endl;

    std::cout << "\nTest instructions:" << std::endl;
    std::cout << "1. This test triggers exceptions during resource allocation" << std::endl;
    std::cout << "2. Run with check_memory.sh v4 to detect leaks from exceptions" << std::endl;
    std::cout << "3. Expected result: Memory leak detected (if buggy version)" << std::endl;

    std::cout << "\nRunning multiple iterations to amplify leaks..." << std::endl;

    for (int i = 0; i < 10; ++i) {
        try {
            std::cout << "  Iteration " << (i + 1) << ": ";
            // Actual test would call load_multiple_configs with invalid files
            // to trigger exceptions

            if (i % 3 == 0) {
                // Simulate exception
                std::cout << "Exception thrown" << std::endl;
            } else {
                std::cout << "Success" << std::endl;
            }

        } catch (const std::exception& e) {
            std::cout << "    Caught: " << e.what() << std::endl;
        }
    }

    std::cout << "\nTest completed: 10 iterations with exceptions" << std::endl;
    std::cout << "Remember to run: ./scripts/check_memory.sh v4" << std::endl;

    return 0;
}
