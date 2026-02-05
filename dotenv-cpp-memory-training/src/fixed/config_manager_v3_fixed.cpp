// Problem 3 FIXED: Dangling Pointer
// This code demonstrates three solutions to avoid returning pointers to local variables

#include <dotenv.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>

// ============================================
// Solution 1: Return std::string by value (Recommended)
// ============================================
#if 1  // Change to 0 to disable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file) : env_filename_(env_file)
    {
        std::cout << "ConfigManager created" << std::endl;
    }

    ~ConfigManager()
    {
        std::cout << "ConfigManager destroyed" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_.c_str());
        std::cout << "Configuration loaded" << std::endl;
    }

    const char* get_value(const char* key)
    {
        return std::getenv(key);
    }

    // FIXED: Return std::string by value
    // Don't worry about performance - RVO (Return Value Optimization) will optimize this
    std::string get_formatted_connection_string()
    {
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");

        char buffer[256];
        if (host && port) {
            std::snprintf(buffer, sizeof(buffer),
                         "postgresql://%s:%s/mydb", host, port);
        } else {
            std::strcpy(buffer, "postgresql://localhost:5432/mydb");
        }

        return std::string(buffer);  // Safe - returns a copy
    }

private:
    std::string env_filename_;
};

#endif

// ============================================
// Solution 2: Use output parameter
// ============================================
#if 0  // Change to 1 to enable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file) : env_filename_(env_file)
    {
        std::cout << "ConfigManager created" << std::endl;
    }

    ~ConfigManager()
    {
        std::cout << "ConfigManager destroyed" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_.c_str());
        std::cout << "Configuration loaded" << std::endl;
    }

    const char* get_value(const char* key)
    {
        return std::getenv(key);
    }

    // FIXED: Use output parameter instead of return value
    void get_formatted_connection_string(std::string& out)
    {
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");

        char buffer[256];
        if (host && port) {
            std::snprintf(buffer, sizeof(buffer),
                         "postgresql://%s:%s/mydb", host, port);
        } else {
            std::strcpy(buffer, "postgresql://localhost:5432/mydb");
        }

        out = buffer;
    }

private:
    std::string env_filename_;
};

#endif

// ============================================
// Solution 3: Return smart pointer (advanced)
// ============================================
#if 0  // Change to 1 to enable this solution

class ConfigManager
{
public:
    ConfigManager(const char* env_file) : env_filename_(env_file)
    {
        std::cout << "ConfigManager created" << std::endl;
    }

    ~ConfigManager()
    {
        std::cout << "ConfigManager destroyed" << std::endl;
    }

    void load_config()
    {
        dotenv::init(env_filename_.c_str());
        std::cout << "Configuration loaded" << std::endl;
    }

    const char* get_value(const char* key)
    {
        return std::getenv(key);
    }

    // FIXED: Return unique_ptr to dynamically allocated string
    // Note: This is overkill for this case, but demonstrates the pattern
    std::unique_ptr<std::string> get_formatted_connection_string()
    {
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");

        char buffer[256];
        if (host && port) {
            std::snprintf(buffer, sizeof(buffer),
                         "postgresql://%s:%s/mydb", host, port);
        } else {
            std::strcpy(buffer, "postgresql://localhost:5432/mydb");
        }

        return std::make_unique<std::string>(buffer);
    }

private:
    std::string env_filename_;
};

#endif

// Helper function to add some stack frames
void some_other_function()
{
    char dummy[100];
    for (int i = 0; i < 100; i++) {
        dummy[i] = 'X';
    }
    std::cout << "Some other function executed" << std::endl;
}

// ============================================
// Main function
// ============================================
int main()
{
    std::cout << "=== Problem 3 FIXED: No Dangling Pointer ===" << std::endl;

    ConfigManager config("../env-files/complex.env");
    config.load_config();

#if 1  // Solution 1: Return by value
    std::string conn_str = config.get_formatted_connection_string();
#elif 0  // Solution 2: Output parameter
    std::string conn_str;
    config.get_formatted_connection_string(conn_str);
#elif 0  // Solution 3: Smart pointer
    auto conn_str_ptr = config.get_formatted_connection_string();
    std::string conn_str = *conn_str_ptr;
#endif

    // Call another function to change the stack
    some_other_function();

    // Now safe to use - conn_str is a proper string object
    std::cout << "\nUsing the connection string safely..." << std::endl;
    std::cout << "Connection: " << conn_str << std::endl;

    std::cout << "\nProgram ending..." << std::endl;
    return 0;
}

// ============================================
// Key Lessons:
// ============================================
// 1. NEVER return pointers or references to local variables
// 2. Local variables are destroyed when function returns
// 3. Return std::string by value is efficient due to RVO (Return Value Optimization)
// 4. Output parameters are clear about ownership but less elegant
// 5. Smart pointers clarify ownership but add complexity
// 6. For simple cases, returning by value is the best choice
// 7. Modern C++ compilers optimize away most copies (move semantics, RVO)
