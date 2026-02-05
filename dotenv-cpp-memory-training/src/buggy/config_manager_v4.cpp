// Problem 4: Exception Safety
// This code demonstrates resource leaks when exceptions are thrown

#include <dotenv.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cstring>

class ConfigManager
{
public:
    ConfigManager() : env_filename_("default.env")
    {
        std::cout << "ConfigManager created" << std::endl;
    }

    ~ConfigManager()
    {
        std::cout << "ConfigManager destroyed" << std::endl;
    }

    // TODO: 如果这里抛出异常会怎样？
    // PROBLEM: Manual resource management with no exception safety
    void load_multiple_configs(const char** files, int count)
    {
        std::cout << "\nLoading " << count << " configuration files..." << std::endl;

        // Allocate array of buffers - MANUAL MEMORY MANAGEMENT
        char** buffers = new char*[count];

        for (int i = 0; i < count; ++i) {
            buffers[i] = new char[1024];  // Allocate buffer for each file

            std::cout << "Processing file " << (i + 1) << ": " << files[i] << std::endl;

            // Try to read the file
            std::ifstream file(files[i]);
            if (!file.is_open()) {
                // DANGER: Exception thrown before cleanup!
                throw std::runtime_error(std::string("Cannot open file: ") + files[i]);
            }

            // Read first line into buffer
            file.getline(buffers[i], 1024);
            file.close();

            std::cout << "  Content preview: " << buffers[i] << std::endl;
        }

        // Process all buffers...
        std::cout << "\nProcessing all configurations..." << std::endl;

        // Cleanup code (only reached if no exception!)
        for (int i = 0; i < count; ++i) {
            delete[] buffers[i];
        }
        delete[] buffers;

        std::cout << "All configurations loaded successfully" << std::endl;
    }

private:
    std::string env_filename_;
};

int main()
{
    std::cout << "=== Problem 4: Exception Safety ===" << std::endl;

    try {
        ConfigManager config;

        // Create test: one file exists, one doesn't
        const char* files[] = {
            "../env-files/basic.env",
            "../env-files/nonexistent.env",  // This will cause exception!
            "../env-files/complex.env"
        };

        // This will throw an exception after allocating some resources
        config.load_multiple_configs(files, 3);

    } catch (const std::exception& e) {
        std::cout << "\n[EXCEPTION CAUGHT] " << e.what() << std::endl;
        std::cout << "[WARNING] Memory leaked! Resources allocated before exception were not freed." << std::endl;
    }

    std::cout << "\nProgram ending..." << std::endl;
    return 0;
}
