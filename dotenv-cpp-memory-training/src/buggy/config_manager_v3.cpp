// Problem 3: Dangling Pointer (Returning pointer to local variable)
// This code demonstrates the problem of returning a pointer to stack memory

#include <dotenv.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

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

    // TODO: 这个返回值安全吗？
    // PROBLEM: Returns pointer to local variable!
    const char* get_formatted_connection_string()
    {
        char buffer[256];  // Local variable on the stack

        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");

        if (host && port) {
            std::snprintf(buffer, sizeof(buffer),
                         "postgresql://%s:%s/mydb", host, port);
        } else {
            std::strcpy(buffer, "postgresql://localhost:5432/mydb");
        }

        // DANGER: Returning pointer to local variable that will be destroyed!
        return buffer;
    }

private:
    std::string env_filename_;
};

// Helper function to add some stack frames
void some_other_function()
{
    char dummy[100];
    for (int i = 0; i < 100; i++) {
        dummy[i] = 'X';  // Overwrite stack memory
    }
    std::cout << "Some other function executed" << std::endl;
}

int main()
{
    std::cout << "=== Problem 3: Dangling Pointer ===" << std::endl;

    ConfigManager config("../env-files/complex.env");
    config.load_config();

    // Get the connection string
    const char* conn_str = config.get_formatted_connection_string();

    // Call another function to change the stack
    some_other_function();

    // Try to use the pointer - UNDEFINED BEHAVIOR!
    // The memory it points to has been destroyed
    std::cout << "\nTrying to use the connection string..." << std::endl;
    std::cout << "Connection: " << conn_str << std::endl;  // Dangling pointer!

    std::cout << "\nProgram ending..." << std::endl;
    return 0;
}
