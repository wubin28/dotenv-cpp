// Problem 2: Double-Free (Shallow Copy)
// This code demonstrates the problem of default shallow copy leading to double-free

#include "config_manager.h"
#include <dotenv.h>
#include <iostream>

ConfigManager::ConfigManager(const char* env_file)
{
    env_filename_ = new char[std::strlen(env_file) + 1];
    std::strcpy(env_filename_, env_file);
    std::cout << "ConfigManager created with file: " << env_filename_ << std::endl;
}

// FIXED from Problem 1: Properly free memory
ConfigManager::~ConfigManager()
{
    if (env_filename_ != nullptr) {
        std::cout << "Freeing memory for: " << env_filename_ << std::endl;
        delete[] env_filename_;
        env_filename_ = nullptr;
    }
    std::cout << "ConfigManager destroyed" << std::endl;
}

// TODO: 这里发生了什么？
// Missing: Copy constructor and copy assignment operator!
// The compiler-generated versions do shallow copy, causing both objects to point to the same memory

void ConfigManager::load_config()
{
    dotenv::init(env_filename_);
    std::cout << "Configuration loaded from " << env_filename_ << std::endl;
}

const char* ConfigManager::get_value(const char* key)
{
    return std::getenv(key);
}

int main()
{
    std::cout << "=== Problem 2: Double-Free (Shallow Copy) ===" << std::endl;

    {
        std::cout << "\nCreating config1..." << std::endl;
        ConfigManager config1("../env-files/basic.env");

        std::cout << "\nCreating config2 from config1 (copy)..." << std::endl;
        ConfigManager config2 = config1;  // Shallow copy! Both point to same memory

        std::cout << "\nLeaving scope - both objects will be destroyed..." << std::endl;
        // When this scope ends:
        // 1. config2 destructor runs - frees the memory
        // 2. config1 destructor runs - tries to free the SAME memory again (double-free!)
    }

    std::cout << "\nProgram ending..." << std::endl;
    return 0;
}
