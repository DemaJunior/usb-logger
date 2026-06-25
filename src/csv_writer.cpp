#include "csv_writer.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <iomanip>
#include <ctime>

std::string CsvWriter::build_path(const LogConfig& cfg) {
    return cfg.directory + "/" + cfg.base_name + current_timestamp_for_filename() + ".csv";
}

CsvWriter::CsvWriter(const LogConfig& cfg)
    : path_(build_path(cfg))
{
    std::filesystem::create_directories(cfg.directory);
    file_.open(path_, std::ios::app);
}

CsvWriter::~CsvWriter() {
    if (file_.is_open()) file_.close();
}

void CsvWriter::write(const std::vector<std::string>& fields) {
    if (!file_.is_open() || fields.empty()) return;
    file_ << current_timestamp();
    for (const auto& f : fields)
        file_ << ',' << f;
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

std::string CsvWriter::current_timestamp_for_filename() {
    using namespace std::chrono;
    
    // Captura o tempo atual de alta precisão
    const auto now = system_clock::now();
    const auto t   = system_clock::to_time_t(now);
    
    // Extrai os milissegundos isolados da fração de segundo atual
    const auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    
    // Buffer local seguro para threads no Linux (localtime_r)
    std::tm buf;
    localtime_r(&t, &buf);

    std::ostringstream ss;
    // O formato %Y%m%d_%H%M%S gera: 20260625_135040 (Sem espaços ou dois-pontos)
    ss << std::put_time(&buf, "%Y%m%d_%H%M%S")
       << '.' << std::setw(3) << std::setfill('0') << ms.count();
       
    return ss.str();
}
