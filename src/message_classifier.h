#pragma once
#include <string>
#include <sstream>
#include <regex>
#include "utils.h"

// Telemetry fields parsed from a valid device line.
struct TelemetryRecord {
    std::string address;
    double voltage_V{};
    double current_A{};
    double energy_Wh{};
    double temperature_C{};
};

// Returns true if `line` matches the format:
//   <address>,<voltage_V>,<current_A>,<energy_Wh>,<temperature_C>
// On success, fills `rec`.
inline bool classify_message(const std::string& line, std::vector<std::string>& rec) {
    // Regex: non-empty address, then 4 numeric fields (int or float, optional sign)

    auto recs = utils::split_string(line, ',');

    if (recs.size() < 5) return false;
    try {
        rec.insert(rec.end(), recs.begin(), recs.end());
        return true;
    } catch (...) {
        return false;
    }
}
