#include "reporter.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_set>

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------

ReadResult read_csv_file(const std::string& path,
                         const std::string& var_name) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {false,
                "[ERROR] Cannot open " + var_name + ": " + path};
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return {true, ss.str()};
}

// ---------------------------------------------------------------------------
// Row parsing helpers
// ---------------------------------------------------------------------------

std::vector<SalesRecord> parse_sales_rows(
    const std::vector<IndexedRow>& all_rows,
    std::ostream& warnings_out) {

    std::vector<SalesRecord> result;
    bool skip_header = true;

    for (const auto& [line_num, row] : all_rows) {
        if (skip_header) {
            skip_header = false;
            continue;
        }
        if (row.size() < 3) {
            warnings_out << "[WARN] Skipping malformed row at line "
                         << line_num
                         << ": insufficient columns\n";
            continue;
        }
        SalesRecord rec;
        rec.order_id   = row[0];
        rec.product_id = row[1];
        try {
            rec.amount = std::stod(row[2]);
        } catch (...) {
            warnings_out << "[WARN] Skipping malformed row at line "
                         << line_num
                         << ": invalid amount value\n";
            continue;
        }
        result.push_back(rec);
    }
    return result;
}

std::vector<InventoryRecord> parse_inventory_rows(
    const std::vector<IndexedRow>& all_rows,
    std::ostream& warnings_out) {

    std::vector<InventoryRecord> result;
    bool skip_header = true;

    for (const auto& [line_num, row] : all_rows) {
        if (skip_header) {
            skip_header = false;
            continue;
        }
        if (row.size() < 2) {
            warnings_out << "[WARN] Skipping malformed row at line "
                         << line_num
                         << ": insufficient columns\n";
            continue;
        }
        InventoryRecord rec;
        rec.product_id = row[0];
        try {
            rec.stock_qty = std::stoi(row[1]);
        } catch (...) {
            warnings_out << "[WARN] Skipping malformed row at line "
                         << line_num
                         << ": invalid stock_qty value\n";
            continue;
        }
        result.push_back(rec);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Join & filter
// ---------------------------------------------------------------------------

std::vector<SalesRecord> join_with_inventory(
    const std::vector<SalesRecord>& sales,
    const std::vector<InventoryRecord>& inventory) {

    std::unordered_set<std::string> product_set;
    for (const auto& inv : inventory) {
        product_set.insert(inv.product_id);
    }

    std::vector<SalesRecord*> temp;
    for (const auto& sale : sales) {
        if (product_set.count(sale.product_id)) {
            temp.push_back(new SalesRecord(sale));
        }
    }

    std::vector<SalesRecord> result;
    for (SalesRecord* p : temp) {
        result.push_back(*p);
    }
    return result;
}

std::vector<SalesRecord> filter_by_amount(
    const std::vector<SalesRecord>& sales,
    double min_amount) {

    std::vector<SalesRecord> result;
    for (const auto& sale : sales) {
        if (sale.amount >= min_amount) {
            result.push_back(sale);
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Aggregation
// ---------------------------------------------------------------------------

Summary compute_summary(const std::vector<SalesRecord>& rows) {
    Summary s;
    s.count = static_cast<int>(rows.size());
    for (const auto& row : rows) {
        s.total += row.amount;
    }
    s.average = (s.count > 0) ? (s.total / s.count) : 0.0;
    return s;
}

// ---------------------------------------------------------------------------
// Formatting
// ---------------------------------------------------------------------------

std::string format_json(const Summary& summary) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"count\": " << summary.count
       << ", \"total\": " << summary.total
       << ", \"average\": " << summary.average << "}";
    return ss.str();
}

std::string format_csv_output(const Summary& summary) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "count,total,average\n";
    ss << summary.count << ","
       << summary.total << ","
       << summary.average << "\n";
    return ss.str();
}
