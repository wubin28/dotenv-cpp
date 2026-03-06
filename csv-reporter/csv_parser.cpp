#include "csv_parser.h"

#include <sstream>

std::vector<IndexedRow> parse_csv_content(const std::string& content,
                                          char delimiter) {
    std::vector<IndexedRow> result;
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;

    while (std::getline(stream, line)) {
        ++line_num;
        // Strip trailing CR (Windows CRLF)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        CsvRow row;
        std::istringstream ls(line);
        std::string field;
        while (std::getline(ls, field, delimiter)) {
            row.push_back(field);
        }
        result.emplace_back(line_num, std::move(row));
    }
    return result;
}
