// Problem 1: Basic Memory Leak
// This code demonstrates a simple memory leak caused by forgetting to free dynamically allocated memory.

#include "config_manager.h"
#include <dotenv.h>
#include <iostream>

ConfigManager::ConfigManager(const char* env_file)
{
    // Allocate memory for the filename
    env_filename_ = new char[std::strlen(env_file) + 1];
    std::strcpy(env_filename_, env_file);

    std::cout << "ConfigManager created with file: " << env_filename_ << std::endl;
}

// TODO: 这里可能有内存管理问题
ConfigManager::~ConfigManager()
{
    // Destructor is empty - no cleanup!
    std::cout << "ConfigManager destroyed" << std::endl;
}

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
    std::cout << "=== Problem 1: Basic Memory Leak ===" << std::endl;

    ConfigManager config("../env-files/basic.env");
    config.load_config();

    const char* db_host = config.get_value("DATABASE_HOST");
    if (db_host) {
        std::cout << "Database host: " << db_host << std::endl;
    }

    std::cout << "Program ending..." << std::endl;
    return 0;
}
