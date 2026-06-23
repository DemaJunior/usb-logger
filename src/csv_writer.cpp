#include "csv_writer.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

CsvWriter::CsvWriter(const std::string& path) {
    bool exists = std::filesystem::exists(path);
    m_file.open(path, std::ios::app);
    if (m_file.is_open() && !exists) {
        m_file << kHeader;
    }
}

CsvWriter::~CsvWriter() {
    if (m_file.is_open()) m_file.close();
}

void CsvWriter::write(const TelemetryRecord& rec) {
    if (!m_file.is_open()) return;
    m_file << current_timestamp() << ','
           << rec.address << ','
           << rec.voltage_V << ','
           << rec.current_A << ','
           << rec.energy_Wh << ','
           << rec.temperature_C << '\n';
    m_file.flush(); // ensure data is written even if process is killed
}

std::string CsvWriter::current_timestamp() const {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t   = system_clock::to_time_t(now);
    auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}
