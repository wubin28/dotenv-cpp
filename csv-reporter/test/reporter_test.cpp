// reporter_test.cpp
//
// TDD iterations covered in this file (in AC order):
//  AC-H1 – join + filter + compute_summary + format_json produce correct output
//  AC-H2 – same logic works with different threshold (south-region scenario)
//  AC-H3 – parse_sales_rows / parse_inventory_rows work with pipe-delimited data
//  AC-H4 – filter_by_amount with threshold above all amounts yields empty summary
//  AC-H5 – format_csv_output produces correct CSV header + data row
//  AC-S2 – read_csv_file returns failure for a non-existent path
//  AC-S3 – parse_sales_rows skips rows with < 3 fields and warns with line number

#include <gtest/gtest.h>

#include "csv_parser.h"
#include "reporter.h"

#include <sstream>
#include <string>

// ---------------------------------------------------------------------------
// Helpers shared across tests
// ---------------------------------------------------------------------------

namespace {

const std::string kSalesCsv =
    "order_id,product_id,amount\n"
    "O001,P001,1500\n"
    "O002,P002,800\n"
    "O003,P001,2000\n"
    "O004,P003,1200\n";

const std::string kInventoryCsv =
    "product_id,stock_qty\n"
    "P001,100\n"
    "P002,200\n"
    "P003,50\n";

std::vector<SalesRecord> make_sales(char delim = ',') {
    std::ostringstream devnull;
    auto rows = parse_csv_content(kSalesCsv, delim);
    return parse_sales_rows(rows, devnull);
}

std::vector<InventoryRecord> make_inventory(char delim = ',') {
    std::ostringstream devnull;
    auto rows = parse_csv_content(kInventoryCsv, delim);
    return parse_inventory_rows(rows, devnull);
}

}  // namespace

// ---------------------------------------------------------------------------
// AC-H1  Full happy-path: join → filter(1000) → summary → JSON
// ---------------------------------------------------------------------------

TEST(JoinWithInventory, AC_H1_JoinKeepsOnlySalesWithMatchingProduct) {
    auto sales     = make_sales();
    auto inventory = make_inventory();

    // All 4 sales have a matching product → 4 joined records
    auto joined = join_with_inventory(sales, inventory);
    ASSERT_EQ(joined.size(), 4u);
}

TEST(FilterByAmount, AC_H1_FilterRemovesRowsBelowThreshold) {
    auto sales     = make_sales();
    auto inventory = make_inventory();

    auto joined   = join_with_inventory(sales, inventory);
    auto filtered = filter_by_amount(joined, 1000.0);

    // O002 (800) is below 1000; expected remaining: O001, O003, O004
    ASSERT_EQ(filtered.size(), 3u);
}

TEST(ComputeSummary, AC_H1_SummaryIsCorrect) {
    auto sales     = make_sales();
    auto inventory = make_inventory();
    auto filtered  = filter_by_amount(join_with_inventory(sales, inventory), 1000.0);

    auto s = compute_summary(filtered);
    EXPECT_EQ(s.count, 3);
    EXPECT_NEAR(s.total,   4700.0,          0.001);
    EXPECT_NEAR(s.average, 4700.0 / 3.0,    0.001);
}

TEST(FormatJson, AC_H1_JsonContainsCountTotalAverage) {
    Summary s;
    s.count   = 3;
    s.total   = 4700.0;
    s.average = 4700.0 / 3.0;

    const std::string json = format_json(s);

    EXPECT_NE(json.find("\"count\": 3"),       std::string::npos);
    EXPECT_NE(json.find("\"total\": 4700.00"), std::string::npos);
    // Average rounds to 1566.67
    EXPECT_NE(json.find("\"average\": 1566.67"), std::string::npos);
}

TEST(FormatJson, AC_H1_JsonStartsAndEndsWithBraces) {
    Summary s{1, 100.0, 100.0};
    const auto json = format_json(s);
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(),  '}');
}

// ---------------------------------------------------------------------------
// AC-H2  Different MIN_AMOUNT (2000) reflects a different region's config
// ---------------------------------------------------------------------------

TEST(FilterByAmount, AC_H2_DifferentThresholdGivesDifferentResult) {
    auto sales     = make_sales();
    auto inventory = make_inventory();
    auto joined    = join_with_inventory(sales, inventory);

    // With 1000: 3 rows survive; with 2000: only O003 (2000) survives
    auto filtered_1000 = filter_by_amount(joined, 1000.0);
    auto filtered_2000 = filter_by_amount(joined, 2000.0);

    EXPECT_EQ(filtered_1000.size(), 3u);
    EXPECT_EQ(filtered_2000.size(), 1u);
    EXPECT_EQ(filtered_2000[0].order_id, "O003");
}

// ---------------------------------------------------------------------------
// AC-H3  Pipe-delimited data is parsed and produces the same records
// ---------------------------------------------------------------------------

TEST(ParseSalesRows, AC_H3_PipeDelimitedSalesParsesCorrectly) {
    const std::string pipe_sales =
        "order_id|product_id|amount\n"
        "O001|P001|1500\n"
        "O002|P002|800\n";

    std::ostringstream devnull;
    auto rows = parse_csv_content(pipe_sales, '|');
    auto sales = parse_sales_rows(rows, devnull);

    ASSERT_EQ(sales.size(), 2u);
    EXPECT_EQ(sales[0].order_id,   "O001");
    EXPECT_EQ(sales[0].product_id, "P001");
    EXPECT_NEAR(sales[0].amount, 1500.0, 0.001);
}

TEST(ParseInventoryRows, AC_H3_PipeDelimitedInventoryParsesCorrectly) {
    const std::string pipe_inventory =
        "product_id|stock_qty\n"
        "P001|100\n";

    std::ostringstream devnull;
    auto rows = parse_csv_content(pipe_inventory, '|');
    auto inv  = parse_inventory_rows(rows, devnull);

    ASSERT_EQ(inv.size(), 1u);
    EXPECT_EQ(inv[0].product_id, "P001");
    EXPECT_EQ(inv[0].stock_qty,   100);
}

// ---------------------------------------------------------------------------
// AC-H4  Threshold above all amounts → empty summary with valid zero values
// ---------------------------------------------------------------------------

TEST(FilterByAmount, AC_H4_ThresholdAboveAllAmountsYieldsEmptyList) {
    auto sales     = make_sales();
    auto inventory = make_inventory();
    auto joined    = join_with_inventory(sales, inventory);
    auto filtered  = filter_by_amount(joined, 9999999.0);

    EXPECT_TRUE(filtered.empty());
}

TEST(ComputeSummary, AC_H4_EmptyInputGivesZeroSummary) {
    const auto s = compute_summary({});
    EXPECT_EQ(s.count, 0);
    EXPECT_DOUBLE_EQ(s.total,   0.0);
    EXPECT_DOUBLE_EQ(s.average, 0.0);
}

TEST(FormatJson, AC_H4_EmptySummaryIsValidJson) {
    const auto json = format_json({0, 0.0, 0.0});
    EXPECT_NE(json.find("\"count\": 0"),    std::string::npos);
    EXPECT_NE(json.find("\"total\": 0.00"), std::string::npos);
}

// ---------------------------------------------------------------------------
// AC-H5  CSV output format: header line + data line, no JSON content
// ---------------------------------------------------------------------------

TEST(FormatCsvOutput, AC_H5_OutputContainsCsvHeaderAndData) {
    Summary s{3, 4700.0, 4700.0 / 3.0};
    const auto output = format_csv_output(s);

    EXPECT_NE(output.find("count,total,average"), std::string::npos);
    EXPECT_NE(output.find("3,4700.00,1566.67"),   std::string::npos);
}

TEST(FormatCsvOutput, AC_H5_OutputDoesNotContainJsonSyntax) {
    const auto output = format_csv_output({3, 4700.0, 4700.0 / 3.0});
    EXPECT_EQ(output.find('{'), std::string::npos);
    EXPECT_EQ(output.find('}'), std::string::npos);
}

// ---------------------------------------------------------------------------
// AC-S2  read_csv_file returns failure for a path that does not exist
// ---------------------------------------------------------------------------

TEST(ReadCsvFile, AC_S2_NonExistentSalesFileReturnsFailure) {
    auto result = read_csv_file("nonexistent/path/file.csv", "SALES_FILE");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.value.find("[ERROR]"),       std::string::npos);
    EXPECT_NE(result.value.find("SALES_FILE"),    std::string::npos);
    EXPECT_NE(result.value.find("nonexistent/path/file.csv"), std::string::npos);
}

TEST(ReadCsvFile, AC_S2_ErrorMessageMentionsVariableName) {
    auto result = read_csv_file("no_such.csv", "INVENTORY_FILE");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.value.find("INVENTORY_FILE"), std::string::npos);
}

// ---------------------------------------------------------------------------
// AC-S3  parse_sales_rows skips a row with too few columns and warns
// ---------------------------------------------------------------------------

TEST(ParseSalesRows, AC_S3_MalformedRowIsSkippedWithLineNumberWarning) {
    // Line 1: header
    // Line 2: valid
    // Line 3: malformed (only 2 columns)
    // Line 4: valid
    const std::string content =
        "order_id,product_id,amount\n"
        "O001,P001,1500\n"
        "O002,P002\n"
        "O003,P001,2000\n";

    auto rows = parse_csv_content(content, ',');

    std::ostringstream warnings;
    auto sales = parse_sales_rows(rows, warnings);

    // Only 2 valid records survive
    ASSERT_EQ(sales.size(), 2u);
    EXPECT_EQ(sales[0].order_id, "O001");
    EXPECT_EQ(sales[1].order_id, "O003");

    // Warning message references the correct line number and diagnostic text
    const auto warn = warnings.str();
    EXPECT_NE(warn.find("[WARN]"),               std::string::npos);
    EXPECT_NE(warn.find("line 3"),               std::string::npos);
    EXPECT_NE(warn.find("insufficient columns"), std::string::npos);
}

TEST(ParseSalesRows, AC_S3_ValidRowsAfterMalformedRowAreStillProcessed) {
    const std::string content =
        "order_id,product_id,amount\n"
        "BAD_ROW\n"
        "O001,P001,500\n";

    auto rows = parse_csv_content(content, ',');

    std::ostringstream warnings;
    auto sales = parse_sales_rows(rows, warnings);

    // The bad row at line 2 is skipped; O001 at line 3 is processed
    ASSERT_EQ(sales.size(), 1u);
    EXPECT_EQ(sales[0].order_id, "O001");
    EXPECT_NE(warnings.str().find("line 2"), std::string::npos);
}
