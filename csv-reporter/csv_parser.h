#pragma once

#include <string>
#include <utility>
#include <vector>

using CsvRow = std::vector<std::string>;

// Line number (1-based) paired with its parsed fields
using IndexedRow = std::pair<int, CsvRow>;

// Splits content into rows. The first row is treated as the header and
// included at index 0. Empty lines are skipped; line numbers reflect the
// original 1-based position in the content string including skipped lines.
std::vector<IndexedRow> parse_csv_content(const std::string& content,
                                          char delimiter);
