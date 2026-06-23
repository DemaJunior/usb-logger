#include "line_logger.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "file_utils.h"

namespace {

static std::string today_suffix() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const std::time_t t = clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

}

LineLogger::LineLogger(LogConfig cfg) : cfg_(std::move(cfg)) {
}

bool LineLogger::reopen_if_needed() {
    if (cfg_.reopen_interval_s <= 0) return true;

    const auto now = std::chrono::steady_clock::now();
    if (now - last_open_ < std::chrono::seconds(cfg_.reopen_interval_s)) return true;

    return open_current_file();
}

bool LineLogger::open_current_file() {
    if (!file_utils::ensure_directory(cfg_.directory)) {
        return false;
    }

    const auto filename = cfg_.base_name + "-" + today_suffix() + ".log";
    const auto path = (std::filesystem::path(cfg_.directory) / filename).string();

    if (out_.is_open() && current_path_ == path) {
        last_open_ = std::chrono::steady_clock::now();
        return true;
    }

    if (out_.is_open()) out_.close();

    out_.open(path, std::ios::out | std::ios::app);
    if (!out_.is_open()) return false;

    current_path_ = path;
    last_open_ = std::chrono::steady_clock::now();
    return true;
}

bool LineLogger::write_line(const std::string& line) {
    if (!out_.is_open()) {
        if (!open_current_file()) return false;
    }

    if (!reopen_if_needed()) return false;

    out_ << line << '\n';
    if (cfg_.flush_each_line) out_.flush();
    return static_cast<bool>(out_);
}
