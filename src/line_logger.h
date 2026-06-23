#pragma once

#include <chrono>
#include <fstream>
#include <string>

#include "app_config.h"

class LineLogger {
public:
    explicit LineLogger(LogConfig cfg);

    bool write_line(const std::string& line);

private:
    bool reopen_if_needed();
    bool open_current_file();

    LogConfig cfg_;
    std::ofstream out_;
    std::chrono::steady_clock::time_point last_open_{std::chrono::steady_clock::now()};
    std::string current_path_;
};
