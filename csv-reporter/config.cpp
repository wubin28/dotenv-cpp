#include "config.h"

#include <dotenv.h>
#include <fstream>

std::variant<Config, std::string> validate_config(
    const std::string& sales_file,
    const std::string& inventory_file,
    const std::string& delimiter_str,
    const std::string& min_amount_str,
    const std::string& output_format_str) {

    Config config;
    config.sales_file = sales_file;
    config.inventory_file = inventory_file;

    // AC-S6: missing DELIMITER defaults to ','
    config.delimiter = delimiter_str.empty() ? ',' : delimiter_str[0];

    // AC-S4: MIN_AMOUNT must be a valid number
    try {
        config.min_amount = std::stod(min_amount_str);
    } catch (...) {
        return "[ERROR] Invalid value for MIN_AMOUNT: '" + min_amount_str +
               "' is not a valid number.";
    }

    // AC-S5: OUTPUT_FORMAT must be "json" or "csv"
    if (output_format_str != "json" && output_format_str != "csv") {
        return "[ERROR] Unsupported OUTPUT_FORMAT: '" + output_format_str +
               "'. Supported values: json, csv.";
    }
    config.output_format = output_format_str;

    return config;
}

std::variant<Config, std::string> load_config(const std::string& env_file) {
    // AC-S1: .env file must exist
    {
        std::ifstream f(env_file);
        if (!f.good()) {
            return "[ERROR] .env file not found. Please create one before running.";
        }
    }

    dotenv::init(env_file.c_str());

    const auto sales_file       = dotenv::getenv("SALES_FILE", "");
    const auto inventory_file   = dotenv::getenv("INVENTORY_FILE", "");
    const auto delimiter_str    = dotenv::getenv("DELIMITER", "");
    const auto min_amount_str   = dotenv::getenv("MIN_AMOUNT", "");
    const auto output_format_str = dotenv::getenv("OUTPUT_FORMAT", "json");

    return validate_config(sales_file, inventory_file, delimiter_str,
                           min_amount_str, output_format_str);
}
