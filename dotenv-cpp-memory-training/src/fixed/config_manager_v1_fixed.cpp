// Problem 1 FIXED: Basic Memory Leak
// This code demonstrates two solutions to fix the memory leak

#include <dotenv.h>
#include <iostream>
#include <string>
#include <cstring>

// ============================================
// Solution 1: Properly implement destructor
// ============================================
#if 1  // Change to 0 to disable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file)
    {
        env_filename_ = new char[std::strlen(env_file) + 1];
        std::strcpy(env_filename_, env_file);
        std::cout << "ConfigManager created with file: " << env_filename_ << std::endl;
    }

    // FIXED: Properly free the allocated memory
    ~ConfigManager()
    {
        if (env_filename_ != nullptr) {
            delete[] env_filename_;
            env_filename_ = nullptr;
        }
        std::cout << "ConfigManager destroyed (memory freed)" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_);
        std::cout << "Configuration loaded from " << env_filename_ << std::endl;
    }

    const char* get_value(const char* key)
    {
        return std::getenv(key);
    }

private:
    char* env_filename_;
};

#endif

// ============================================
// Solution 2: Use std::string (Recommended)
// ============================================
#if 0  // Change to 1 to enable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file) : env_filename_(env_file)
    {
        // No manual memory allocation needed!
        std::cout << "ConfigManager created with file: " << env_filename_ << std::endl;
    }

    // FIXED: No manual cleanup needed - std::string handles it automatically (RAII)
    ~ConfigManager()
    {
        std::cout << "ConfigManager destroyed (std::string automatic cleanup)" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_.c_str());
        std::cout << "Configuration loaded from " << env_filename_ << std::endl;
    }

    const char* get_value(const char* key)
    {
        return std::getenv(key);
    }

private:
    std::string env_filename_;  // FIXED: Use std::string instead of char*
};

#endif

// ============================================
// Main function
// ============================================
int main()
{
    std::cout << "=== Problem 1 FIXED: No Memory Leak ===" << std::endl;

    ConfigManager config("../env-files/basic.env");
    config.load_config();

    const char* db_host = config.get_value("DATABASE_HOST");
    if (db_host) {
        std::cout << "Database host: " << db_host << std::endl;
    }

    std::cout << "Program ending..." << std::endl;
    return 0;
}

// ============================================
// Key Lessons:
// ============================================
// 1. Every "new" must have a corresponding "delete"
// 2. Use "delete[]" for arrays allocated with "new[]"
// 3. Set pointers to nullptr after deletion (defensive programming)
// 4. Better solution: Use std::string or other RAII types to avoid manual memory management
// 5. RAII (Resource Acquisition Is Initialization) is a fundamental C++ idiom
