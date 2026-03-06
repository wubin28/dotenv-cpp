// csv_parser_test.cpp
//
// TDD iterations covered in this file (in AC order):
//  AC-H1 – comma-delimited CSV parses correctly (drives parse_csv_content)
//  AC-H3 – pipe-delimited CSV parses identically to comma-delimited data
//  AC-S3 – row with fewer columns than the header is still returned (the
//           business-logic layer in reporter_test.cpp tests the skip/warn)

#include <gtest/gtest.h>

#include "csv_parser.h"

// ---------------------------------------------------------------------------
// AC-H1  Standard comma-separated parsing
// ---------------------------------------------------------------------------

TEST(ParseCsvContent, AC_H1_HeaderAndDataRowsParsedWithCommaDelimiter) {
    const std::string content =
        "order_id,product_id,amount\n"
        "O001,P001,1500\n"
        "O002,P002,800\n";

    auto rows = parse_csv_content(content, ',');

    ASSERT_EQ(rows.size(), 3u);

    // Header row at line 1
    EXPECT_EQ(rows[0].first, 1);
    EXPECT_EQ(rows[0].second, (CsvRow{"order_id", "product_id", "amount"}));

    // First data row at line 2
    EXPECT_EQ(rows[1].first, 2);
    EXPECT_EQ(rows[1].second, (CsvRow{"O001", "P001", "1500"}));

    // Second data row at line 3
    EXPECT_EQ(rows[2].first, 3);
    EXPECT_EQ(rows[2].second, (CsvRow{"O002", "P002", "800"}));
}

TEST(ParseCsvContent, AC_H1_EmptyLinesAreSkipped) {
    const std::string content =
        "order_id,product_id,amount\n"
        "\n"
        "O001,P001,1500\n";

    auto rows = parse_csv_content(content, ',');

    ASSERT_EQ(rows.size(), 2u);
    // The empty line is still counted (line 2), so the data row is at line 3
    EXPECT_EQ(rows[0].first, 1);
    EXPECT_EQ(rows[1].first, 3);
}

// ---------------------------------------------------------------------------
// AC-H3  Pipe-separated parsing produces identical field values
// ---------------------------------------------------------------------------

TEST(ParseCsvContent, AC_H3_PipeDelimiterParsesFieldsCorrectly) {
    const std::string content =
        "order_id|product_id|amount\n"
        "O001|P001|1500\n"
        "O002|P002|800\n";

    auto rows = parse_csv_content(content, '|');

    ASSERT_EQ(rows.size(), 3u);
    EXPECT_EQ(rows[0].second, (CsvRow{"order_id", "product_id", "amount"}));
    EXPECT_EQ(rows[1].second, (CsvRow{"O001", "P001", "1500"}));
    EXPECT_EQ(rows[2].second, (CsvRow{"O002", "P002", "800"}));
}

TEST(ParseCsvContent, AC_H3_PipeAndCommaResultsAreEquivalent) {
    const std::string comma_content =
        "order_id,product_id,amount\n"
        "O001,P001,1500\n";
    const std::string pipe_content =
        "order_id|product_id|amount\n"
        "O001|P001|1500\n";

    const auto comma_rows = parse_csv_content(comma_content, ',');
    const auto pipe_rows  = parse_csv_content(pipe_content,  '|');

    ASSERT_EQ(comma_rows.size(), pipe_rows.size());
    for (std::size_t i = 0; i < comma_rows.size(); ++i) {
        EXPECT_EQ(comma_rows[i].second, pipe_rows[i].second);
    }
}

// ---------------------------------------------------------------------------
// AC-S3  Malformed rows are still returned by the parser; the row-level
//        skip/warn logic lives in parse_sales_rows (reporter_test.cpp).
// ---------------------------------------------------------------------------

TEST(ParseCsvContent, AC_S3_MalformedRowsAreIncludedWithCorrectLineNumber) {
    // Row at original line 3 has only 2 fields instead of 3
    const std::string content =
        "order_id,product_id,amount\n"
        "O001,P001,1500\n"
        "O002,P002\n"
        "O003,P001,2000\n";

    auto rows = parse_csv_content(content, ',');

    ASSERT_EQ(rows.size(), 4u);

    // The malformed row is at line 3 and has 2 fields
    EXPECT_EQ(rows[2].first,          3);
    EXPECT_EQ(rows[2].second.size(),  2u);
    EXPECT_EQ(rows[2].second[0],      "O002");
    EXPECT_EQ(rows[2].second[1],      "P002");
}
