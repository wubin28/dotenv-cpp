#include "config.h"
#include "csv_parser.h"
#include "reporter.h"

#include <iostream>
#include <variant>

int main() {
    auto config_result = load_config();
    if (auto* err = std::get_if<std::string>(&config_result)) {
        std::cerr << *err << "\n";
        return 1;
    }
    const auto& config = std::get<Config>(config_result);

    auto sales_result = read_csv_file(config.sales_file, "SALES_FILE");
    if (!sales_result.success) {
        std::cerr << sales_result.value << "\n";
        return 1;
    }

    auto inventory_result = read_csv_file(config.inventory_file, "INVENTORY_FILE");
    if (!inventory_result.success) {
        std::cerr << inventory_result.value << "\n";
        return 1;
    }

    auto sales_rows     = parse_csv_content(sales_result.value,     config.delimiter);
    auto inventory_rows = parse_csv_content(inventory_result.value, config.delimiter);

    auto sales     = parse_sales_rows(sales_rows,     std::cerr);
    auto inventory = parse_inventory_rows(inventory_rows, std::cerr);

    auto joined   = join_with_inventory(sales, inventory);
    auto filtered = filter_by_amount(joined, config.min_amount);
    auto summary  = compute_summary(filtered);

    if (config.output_format == "json") {
        std::cout << format_json(summary) << "\n";
    } else {
        std::cout << format_csv_output(summary);
    }

    return 0;
}
