// Copyright (c) 2024 dotenv-cpp Memory Training Project
//
// This file is part of the C++ Memory Safety training materials.
// It demonstrates configuration management using dotenv-cpp library.
//
// This software is provided for educational purposes.

#pragma once

#include <string>
#include <cstring>

/**
 * @brief Configuration manager class
 *
 * This class manages application configuration by loading environment
 * variables from .env files using the dotenv-cpp library.
 *
 * NOTE: Different versions of this class contain intentional memory
 * management issues for educational purposes. See the corresponding
 * documentation for details.
 */
class ConfigManager
{
public:
    /**
     * @brief Construct a new Config Manager object
     * @param env_file Path to the .env file to load
     */
    ConfigManager(const char* env_file);

    /**
     * @brief Destroy the Config Manager object
     *
     * NOTE: Implementation varies by version to demonstrate different
     * memory management scenarios.
     */
    ~ConfigManager();

    /**
     * @brief Load configuration from the .env file
     *
     * This method calls dotenv::init() to load environment variables.
     */
    void load_config();

    /**
     * @brief Get the value of a configuration key
     * @param key The configuration key to lookup
     * @return const char* The value, or nullptr if not found
     */
    const char* get_value(const char* key);

private:
    char* env_filename_;  // Stores the .env file name (potential memory issue!)
};
