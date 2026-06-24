#include "csv_writer.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

std::string CsvWriter::build_path(const LogConfig& cfg) {
    return cfg.directory + "/" + cfg.base_name + ".csv";
}

CsvWriter::CsvWriter(const LogConfig& cfg)
    : path_(build_path(cfg))
{
    std::filesystem::create_directories(cfg.directory);
    const bool exists = std::filesystem::exists(path_);
    file_.open(path_, std::ios::app);
    if (file_.is_open() && !exists)
        file_ << kHeader;
}

CsvWriter::~CsvWriter() {
    if (file_.is_open()) file_.close();
}

void CsvWriter::write(const std::vector<std::string>& recs) {
    if (!file_.is_open()) return;
    file_ << current_timestamp();
    for (const auto& rec : recs) {
        file_ << ',' << rec;
    }
    file_ << '\n';
    if (file_.good()) file_.flush();
}

std::string CsvWriter::current_timestamp() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto t   = system_clock::to_time_t(now);
    const auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}
