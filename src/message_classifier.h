#pragma once

#include <string>
#include <vector>
#include "utils.h"

// Returns true if `line` is a valid telemetry line:
//   <address>,<field1>,<field2>,...  (minimum 5 comma-separated fields)
// The format <addr>,<V>,<A>,<Wh>,<T> is the baseline, but any line with
// 5 or more fields is accepted and forwarded to the CSV as-is.
// On success, fills `fields` with all parsed tokens.
// On failure, `fields` is left unchanged.
inline bool classify_message(const std::string& line, std::vector<std::string>& fields) {
    auto tokens = utils::split_string(line, ',');
    if (tokens.size() < 5) return false;
    fields = std::move(tokens);
    return true;
}
