#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "app_config.h"

// Appends rows to a CSV file.
// Path is derived from LogConfig: <directory>/<base_name>.csv
// Creates the file without a fixed header (field count is dynamic).
class CsvWriter {
public:
    explicit CsvWriter(const LogConfig& cfg);
    ~CsvWriter();

    bool        is_open() const { return file_.is_open(); }
    std::string path()    const { return path_; }

    // Writes: timestamp,field0,field1,...,fieldN
    void write(const std::vector<std::string>& fields);

private:
    std::string   path_;
    std::ofstream file_;

    static std::string build_path(const LogConfig& cfg);
    static std::string current_timestamp();
    static std::string current_timestamp_for_filename();
};
