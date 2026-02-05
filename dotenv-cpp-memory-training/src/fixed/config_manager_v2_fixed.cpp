// Problem 2 FIXED: Double-Free (Shallow Copy)
// This code demonstrates three solutions to fix the shallow copy problem

#include <dotenv.h>
#include <iostream>
#include <string>
#include <cstring>

// ============================================
// Solution 1: Implement Rule of Three
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

    // FIXED: Copy constructor (deep copy)
    ConfigManager(const ConfigManager& other)
    {
        env_filename_ = new char[std::strlen(other.env_filename_) + 1];
        std::strcpy(env_filename_, other.env_filename_);
        std::cout << "ConfigManager copy-constructed with file: " << env_filename_ << std::endl;
    }

    // FIXED: Copy assignment operator (deep copy)
    ConfigManager& operator=(const ConfigManager& other)
    {
        if (this != &other) {  // Check for self-assignment
            // Free old memory
            delete[] env_filename_;

            // Allocate new memory and copy
            env_filename_ = new char[std::strlen(other.env_filename_) + 1];
            std::strcpy(env_filename_, other.env_filename_);

            std::cout << "ConfigManager copy-assigned with file: " << env_filename_ << std::endl;
        }
        return *this;
    }

    ~ConfigManager()
    {
        if (env_filename_ != nullptr) {
            std::cout << "Freeing memory for: " << env_filename_ << std::endl;
            delete[] env_filename_;
            env_filename_ = nullptr;
        }
        std::cout << "ConfigManager destroyed" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_);
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
// Solution 2: Delete copy operations
// ============================================
#if 0  // Change to 1 to enable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file)
    {
        env_filename_ = new char[std::strlen(env_file) + 1];
        std::strcpy(env_filename_, env_file);
        std::cout << "ConfigManager created with file: " << env_filename_ << std::endl;
    }

    // FIXED: Explicitly delete copy operations
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    ~ConfigManager()
    {
        if (env_filename_ != nullptr) {
            std::cout << "Freeing memory for: " << env_filename_ << std::endl;
            delete[] env_filename_;
            env_filename_ = nullptr;
        }
        std::cout << "ConfigManager destroyed" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_);
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
// Solution 3: Use std::string (Recommended)
// ============================================
#if 0  // Change to 1 to enable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file) : env_filename_(env_file)
    {
        std::cout << "ConfigManager created with file: " << env_filename_ << std::endl;
    }

    // FIXED: No need to define copy operations - compiler-generated ones work correctly!
    // std::string has proper deep copy semantics built-in

    ~ConfigManager()
    {
        std::cout << "ConfigManager destroyed (std::string automatic cleanup)" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_.c_str());
    }

    const char* get_value(const char* key)
    {
        return std::getenv(key);
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
    std::cout << "=== Problem 2 FIXED: No Double-Free ===" << std::endl;

    {
        std::cout << "\nCreating config1..." << std::endl;
        ConfigManager config1("../env-files/basic.env");

        std::cout << "\nCreating config2 from config1 (copy)..." << std::endl;
        ConfigManager config2 = config1;  // Now safe - proper deep copy

        std::cout << "\nLeaving scope - both objects will be destroyed safely..." << std::endl;
    }

    std::cout << "\nProgram ending..." << std::endl;
    return 0;
}

// ============================================
// Key Lessons:
// ============================================
// 1. Rule of Three: If you define destructor, also define copy constructor and copy assignment
// 2. Rule of Five (C++11): Add move constructor and move assignment for efficiency
// 3. Rule of Zero: Use RAII types (std::string, std::vector) to avoid manual memory management
// 4. Always check for self-assignment in assignment operator (if (this != &other))
// 5. Default copy is shallow - be careful with classes managing resources
// 6. Use "= delete" to explicitly prevent copying when appropriate
