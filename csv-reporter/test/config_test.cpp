// config_test.cpp
//
// TDD iterations covered in this file (in AC order):
//  AC-H1 – validate_config with all valid values returns a Config
//  AC-H2 – validate_config with different valid values also returns a Config
//           (same code path – proves the program adapts to any valid .env)
//  AC-S1 – load_config with a non-existent file returns a "[ERROR]" string
//  AC-S4 – validate_config with non-numeric MIN_AMOUNT returns "[ERROR]"
//  AC-S5 – validate_config with unsupported OUTPUT_FORMAT returns "[ERROR]"
//  AC-S6 – validate_config with empty DELIMITER defaults to ','

#include <gtest/gtest.h>

#include "config.h"

#include <variant>

// ---------------------------------------------------------------------------
// AC-H1  Valid config produces a Config struct
// ---------------------------------------------------------------------------

TEST(ValidateConfig, AC_H1_AllValidValuesReturnConfig) {
    auto result = validate_config(
        "csv-reporter/data/sales.csv",
        "csv-reporter/data/inventory.csv",
        ",", "1000", "json");

    ASSERT_TRUE(std::holds_alternative<Config>(result));

    const auto& cfg = std::get<Config>(result);
    EXPECT_EQ(cfg.sales_file,     "csv-reporter/data/sales.csv");
    EXPECT_EQ(cfg.inventory_file, "csv-reporter/data/inventory.csv");
    EXPECT_EQ(cfg.delimiter,      ',');
    EXPECT_DOUBLE_EQ(cfg.min_amount, 1000.0);
    EXPECT_EQ(cfg.output_format,  "json");
}

// ---------------------------------------------------------------------------
// AC-H2  Switching config values (e.g. south region, lower threshold)
//        uses the same code path – the program needs no recompilation.
// ---------------------------------------------------------------------------

TEST(ValidateConfig, AC_H2_DifferentValidValuesAlsoReturnConfig) {
    auto result = validate_config(
        "csv-reporter/data/south_sales.csv",
        "csv-reporter/data/inventory.csv",
        ",", "2000", "json");

    ASSERT_TRUE(std::holds_alternative<Config>(result));

    const auto& cfg = std::get<Config>(result);
    EXPECT_EQ(cfg.sales_file,        "csv-reporter/data/south_sales.csv");
    EXPECT_DOUBLE_EQ(cfg.min_amount, 2000.0);
}

// ---------------------------------------------------------------------------
// AC-S1  Missing .env file is detected before dotenv::init is called
// ---------------------------------------------------------------------------

TEST(LoadConfig, AC_S1_NonExistentEnvFileReturnsError) {
    auto result = load_config("definitely_nonexistent_path/.env.missing");

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    const auto& err = std::get<std::string>(result);
    EXPECT_NE(err.find("[ERROR]"),            std::string::npos);
    EXPECT_NE(err.find(".env file not found"), std::string::npos);
}

// ---------------------------------------------------------------------------
// AC-S4  Non-numeric MIN_AMOUNT stops the program with a clear error
// ---------------------------------------------------------------------------

TEST(ValidateConfig, AC_S4_NonNumericMinAmountReturnsError) {
    auto result = validate_config("s.csv", "i.csv", ",", "abc", "json");

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    const auto& err = std::get<std::string>(result);
    EXPECT_NE(err.find("[ERROR]"),       std::string::npos);
    EXPECT_NE(err.find("MIN_AMOUNT"),    std::string::npos);
    EXPECT_NE(err.find("abc"),           std::string::npos);
    EXPECT_NE(err.find("not a valid number"), std::string::npos);
}

TEST(ValidateConfig, AC_S4_EmptyMinAmountReturnsError) {
    auto result = validate_config("s.csv", "i.csv", ",", "", "json");

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    const auto& err = std::get<std::string>(result);
    EXPECT_NE(err.find("[ERROR]"), std::string::npos);
}

// ---------------------------------------------------------------------------
// AC-S5  Unsupported OUTPUT_FORMAT prints supported values in the error
// ---------------------------------------------------------------------------

TEST(ValidateConfig, AC_S5_UnknownOutputFormatReturnsError) {
    auto result = validate_config("s.csv", "i.csv", ",", "1000", "xml");

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    const auto& err = std::get<std::string>(result);
    EXPECT_NE(err.find("[ERROR]"),         std::string::npos);
    EXPECT_NE(err.find("OUTPUT_FORMAT"),   std::string::npos);
    EXPECT_NE(err.find("xml"),             std::string::npos);
    EXPECT_NE(err.find("json"),            std::string::npos);
    EXPECT_NE(err.find("csv"),             std::string::npos);
}

// ---------------------------------------------------------------------------
// AC-S6  Missing DELIMITER defaults to ','
// ---------------------------------------------------------------------------

TEST(ValidateConfig, AC_S6_EmptyDelimiterDefaultsToComma) {
    auto result = validate_config("s.csv", "i.csv", "", "500", "json");

    ASSERT_TRUE(std::holds_alternative<Config>(result));
    EXPECT_EQ(std::get<Config>(result).delimiter, ',');
}

TEST(ValidateConfig, AC_S6_ExplicitCommaDelimiterPreserved) {
    auto result = validate_config("s.csv", "i.csv", ",", "500", "csv");

    ASSERT_TRUE(std::holds_alternative<Config>(result));
    EXPECT_EQ(std::get<Config>(result).delimiter, ',');
}
