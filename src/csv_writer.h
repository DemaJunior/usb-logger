#pragma once
#include <fstream>
#include <string>
#include "app_config.h"
#include "message_classifier.h"

// Appends telemetry records to a CSV file.
// Path is derived from LogConfig: <directory>/<base_name>.csv
// Creates the file with a header row on first use.
class CsvWriter {
public:
    explicit CsvWriter(const LogConfig& cfg);
    ~CsvWriter();

    bool is_open() const { return file_.is_open(); }
    std::string path()  const { return path_; }

    void write(const std::vector<std::string>& recs);

private:
    std::string   path_;
    std::ofstream file_;

    static constexpr const char* kHeader =
        "timestamp,address,voltage_V,current_A,energy_Wh,temperature_C\n";

    static std::string build_path(const LogConfig& cfg);
    static std::string current_timestamp();
};
