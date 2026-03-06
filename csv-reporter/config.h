#pragma once

#include <string>
#include <variant>

struct Config {
    std::string sales_file;
    std::string inventory_file;
    char delimiter = ',';
    double min_amount = 0.0;
    std::string output_format = "json";
};

// Validates raw string values obtained from environment variables.
// Returns a Config on success or an "[ERROR] ..." message string on failure.
// AC-S4: min_amount_str must be a valid number.
// AC-S5: output_format_str must be "json" or "csv".
// AC-S6: empty delimiter_str defaults to ','.
std::variant<Config, std::string> validate_config(
    const std::string& sales_file,
    const std::string& inventory_file,
    const std::string& delimiter_str,
    const std::string& min_amount_str,
    const std::string& output_format_str);

// Loads configuration by reading the given .env file via dotenv, then calls
// validate_config.  Returns error string on any failure.
// AC-S1: returns error if env_file does not exist.
std::variant<Config, std::string> load_config(
    const std::string& env_file = ".env");
