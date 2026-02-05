// Problem 4 FIXED: Exception Safety
// This code demonstrates RAII-based solutions for exception safety

#include <dotenv.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <memory>

// ============================================
// Solution 1: Use std::vector with smart pointers (Recommended)
// ============================================
#if 1  // Change to 0 to disable this solution

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

    // FIXED: Use RAII containers - automatic cleanup on exception
    void load_multiple_configs(const char** files, int count)
    {
        std::cout << "\nLoading " << count << " configuration files..." << std::endl;

        // FIXED: Use vector of unique_ptr - automatic cleanup!
        std::vector<std::unique_ptr<char[]>> buffers;

        for (int i = 0; i < count; ++i) {
            // Allocate buffer using smart pointer
            buffers.push_back(std::make_unique<char[]>(1024));

            std::cout << "Processing file " << (i + 1) << ": " << files[i] << std::endl;

            // Try to read the file
            std::ifstream file(files[i]);
            if (!file.is_open()) {
                // Exception thrown - but all previously allocated buffers
                // will be automatically freed by vector + unique_ptr!
                throw std::runtime_error(std::string("Cannot open file: ") + files[i]);
            }

            // Read first line into buffer
            file.getline(buffers.back().get(), 1024);
            file.close();

            std::cout << "  Content preview: " << buffers.back().get() << std::endl;
        }

        // Process all buffers...
        std::cout << "\nProcessing all configurations..." << std::endl;

        // NO MANUAL CLEANUP NEEDED - RAII handles it automatically!
        std::cout << "All configurations loaded successfully" << std::endl;
    }

private:
    std::string env_filename_;
};

#endif

// ============================================
// Solution 2: Use std::vector<std::string> (Even better!)
// ============================================
#if 0  // Change to 1 to enable this solution

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

    // FIXED: Use std::string directly - simplest and safest
    void load_multiple_configs(const char** files, int count)
    {
        std::cout << "\nLoading " << count << " configuration files..." << std::endl;

        std::vector<std::string> buffers;

        for (int i = 0; i < count; ++i) {
            std::cout << "Processing file " << (i + 1) << ": " << files[i] << std::endl;

            std::ifstream file(files[i]);
            if (!file.is_open()) {
                throw std::runtime_error(std::string("Cannot open file: ") + files[i]);
            }

            // Read into string - automatic memory management
            std::string content;
            std::getline(file, content);
            buffers.push_back(content);

            std::cout << "  Content preview: " << buffers.back() << std::endl;
        }

        std::cout << "\nProcessing all configurations..." << std::endl;
        std::cout << "All configurations loaded successfully" << std::endl;
    }

private:
    std::string env_filename_;
};

#endif

// ============================================
// Solution 3: Custom RAII wrapper (Educational)
// ============================================
#if 0  // Change to 1 to enable this solution

// RAII wrapper for buffer management
class BufferGuard
{
public:
    BufferGuard(size_t size) : buffer_(new char[size]), size_(size)
    {
        std::cout << "  [BufferGuard] Allocated " << size << " bytes" << std::endl;
    }

    ~BufferGuard()
    {
        delete[] buffer_;
        std::cout << "  [BufferGuard] Freed " << size_ << " bytes" << std::endl;
    }

    // Prevent copying
    BufferGuard(const BufferGuard&) = delete;
    BufferGuard& operator=(const BufferGuard&) = delete;

    // Allow moving
    BufferGuard(BufferGuard&& other) noexcept
        : buffer_(other.buffer_), size_(other.size_)
    {
        other.buffer_ = nullptr;
        other.size_ = 0;
    }

    char* get() { return buffer_; }

private:
    char* buffer_;
    size_t size_;
};

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

    void load_multiple_configs(const char** files, int count)
    {
        std::cout << "\nLoading " << count << " configuration files..." << std::endl;

        std::vector<BufferGuard> buffers;

        for (int i = 0; i < count; ++i) {
            buffers.emplace_back(1024);

            std::cout << "Processing file " << (i + 1) << ": " << files[i] << std::endl;

            std::ifstream file(files[i]);
            if (!file.is_open()) {
                throw std::runtime_error(std::string("Cannot open file: ") + files[i]);
            }

            file.getline(buffers.back().get(), 1024);
            std::cout << "  Content preview: " << buffers.back().get() << std::endl;
        }

        std::cout << "\nProcessing all configurations..." << std::endl;
        std::cout << "All configurations loaded successfully" << std::endl;
    }

private:
    std::string env_filename_;
};

#endif

// ============================================
// Main function
// ============================================
int main()
{
    std::cout << "=== Problem 4 FIXED: Exception Safe ===" << std::endl;

    try {
        ConfigManager config;

        const char* files[] = {
            "../env-files/basic.env",
            "../env-files/nonexistent.env",
            "../env-files/complex.env"
        };

        config.load_multiple_configs(files, 3);

    } catch (const std::exception& e) {
        std::cout << "\n[EXCEPTION CAUGHT] " << e.what() << std::endl;
        std::cout << "[SUCCESS] No memory leaked! RAII cleaned up all resources automatically." << std::endl;
    }

    std::cout << "\nProgram ending..." << std::endl;
    return 0;
}

// ============================================
// Key Lessons:
// ============================================
// 1. RAII (Resource Acquisition Is Initialization) is the C++ way to guarantee cleanup
// 2. Use std::vector, std::string, and smart pointers instead of manual memory management
// 3. Resources acquired in constructors, released in destructors
// 4. Destructors are called automatically, even when exceptions are thrown
// 5. Stack unwinding ensures all objects in scope are destroyed
// 6. Never write manual cleanup code - use RAII classes instead
// 7. std::unique_ptr, std::shared_ptr, std::vector, std::string are all RAII types
// 8. Exception safety comes naturally with RAII - no special code needed
