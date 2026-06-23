#pragma once
#include <string>
#include <sstream>
#include <regex>

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
inline bool classify_message(const std::string& line, TelemetryRecord& rec) {
    // Regex: non-empty address, then 4 numeric fields (int or float, optional sign)
    static const std::regex kFmt(
        R"(^([^,]+),([+-]?\d+(?:\.\d*)?),([+-]?\d+(?:\.\d*)?),([+-]?\d+(?:\.\d*)?),([+-]?\d+(?:\.\d*)?)$)"
    );
    std::smatch m;
    if (!std::regex_match(line, m, kFmt)) return false;
    try {
        rec.address      = m[1].str();
        rec.voltage_V    = std::stod(m[2].str());
        rec.current_A    = std::stod(m[3].str());
        rec.energy_Wh    = std::stod(m[4].str());
        rec.temperature_C = std::stod(m[5].str());
        return true;
    } catch (...) {
        return false;
    }
}
