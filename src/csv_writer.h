#pragma once
#include <fstream>
#include <string>
#include "message_classifier.h"

// Appends telemetry records to a CSV file.
// Creates the file and writes the header if it does not exist.
class CsvWriter {
public:
    explicit CsvWriter(const std::string& path);
    ~CsvWriter();

    // Returns false if the file could not be opened.
    bool is_open() const { return m_file.is_open(); }

    void write(const TelemetryRecord& rec);

private:
    std::ofstream m_file;
    static constexpr const char* kHeader =
        "timestamp,address,voltage_V,current_A,energy_Wh,temperature_C\n";
    std::string current_timestamp() const;
};
