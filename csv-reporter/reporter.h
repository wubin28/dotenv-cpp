#pragma once

#include "csv_parser.h"

#include <ostream>
#include <string>
#include <vector>

struct SalesRecord {
    std::string order_id;
    std::string product_id;
    double amount = 0.0;
};

struct InventoryRecord {
    std::string product_id;
    int stock_qty = 0;
};

struct Summary {
    int count = 0;
    double total = 0.0;
    double average = 0.0;
};

// Holds the result of attempting to read a file.
struct ReadResult {
    bool success = false;
    std::string value;  // file content on success; "[ERROR] ..." on failure
};

// AC-S2: tries to open and read the file at path.
// var_name is the .env variable name (used in the error message).
ReadResult read_csv_file(const std::string& path,
                         const std::string& var_name);

// AC-S3: Converts indexed rows (including header as the first element) to
// SalesRecord list.  Rows with fewer than 3 fields are skipped; a
// "[WARN] Skipping malformed row at line N: insufficient columns" message is
// written to warnings_out for each such row.
std::vector<SalesRecord> parse_sales_rows(
    const std::vector<IndexedRow>& all_rows,
    std::ostream& warnings_out);

// Converts indexed rows (including header) to InventoryRecord list.
// Malformed rows are skipped with a warning.
std::vector<InventoryRecord> parse_inventory_rows(
    const std::vector<IndexedRow>& all_rows,
    std::ostream& warnings_out);

// AC-H1 / AC-H2: Inner join — returns only those sales records whose
// product_id exists in the inventory list.
std::vector<SalesRecord> join_with_inventory(
    const std::vector<SalesRecord>& sales,
    const std::vector<InventoryRecord>& inventory);

// AC-H1 / AC-H4: Keeps only rows with amount >= min_amount.
std::vector<SalesRecord> filter_by_amount(
    const std::vector<SalesRecord>& sales,
    double min_amount);

// Computes aggregate summary (count, total, average) from a list of records.
Summary compute_summary(const std::vector<SalesRecord>& rows);

// AC-H1: Formats summary as a single-line JSON object.
std::string format_json(const Summary& summary);

// AC-H5: Formats summary as a two-line CSV (header + data row).
std::string format_csv_output(const Summary& summary);
