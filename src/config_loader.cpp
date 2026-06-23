#include "config_loader.h"

#include <fstream>
#include <sstream>
#include <string>

namespace {

static std::string trim(std::string s) {
    auto is_ws = [](unsigned char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    while (!s.empty() && is_ws(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && is_ws(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

static bool parse_bool(const std::string& v, bool& out) {
    if (v == "true" || v == "True" || v == "1") { out = true; return true; }
    if (v == "false" || v == "False" || v == "0") { out = false; return true; }
    return false;
}

static bool parse_int(const std::string& v, int& out) {
    try {
        size_t idx = 0;
        int val = std::stoi(v, &idx, 10);
        if (idx != v.size()) return false;
        out = val;
        return true;
    }
    catch (...) {
        return false;
    }
}

static std::string unquote(std::string v) {
    v = trim(std::move(v));
    if (v.size() >= 2 && ((v.front() == '\"' && v.back() == '\"') || (v.front() == '\'' && v.back() == '\''))) {
        return v.substr(1, v.size() - 2);
    }
    return v;
}

}

std::optional<AppConfig> ConfigLoader::load(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return std::nullopt;

    AppConfig cfg;
    cfg.config_path = path;

    enum class Section { none, serial, log, daemon } section = Section::none;

    std::string line;
    while (std::getline(in, line)) {
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        line = trim(std::move(line));
        if (line.empty()) continue;

        if (line.size() >= 3 && line.front() == '[' && line.back() == ']') {
            const auto sec = line.substr(1, line.size() - 2);
            if (sec == "serial") section = Section::serial;
            else if (sec == "log") section = Section::log;
            else if (sec == "daemon") section = Section::daemon;
            else section = Section::none;
            continue;
        }

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        auto key = trim(line.substr(0, eq));
        auto val_raw = trim(line.substr(eq + 1));
        auto val = unquote(val_raw);

        switch (section) {
        case Section::serial:
            if (key == "port") cfg.serial.port = val;
            else if (key == "baudrate") parse_int(val, cfg.serial.baudrate);
            else if (key == "parity") cfg.serial.parity = val;
            else if (key == "databits") parse_int(val, cfg.serial.databits);
            else if (key == "stopbits") parse_int(val, cfg.serial.stopbits);
            else if (key == "read_timeout_ms") parse_int(val, cfg.serial.read_timeout_ms);
            break;
        case Section::log:
            if (key == "directory") cfg.log.directory = val;
            else if (key == "base_name") cfg.log.base_name = val;
            else if (key == "reopen_interval_s") parse_int(val, cfg.log.reopen_interval_s);
            else if (key == "flush_each_line") parse_bool(val, cfg.log.flush_each_line);
            break;
        case Section::daemon:
            if (key == "retry_port_interval_s") parse_int(val, cfg.daemon.retry_port_interval_s);
            else if (key == "retry_error_interval_s") parse_int(val, cfg.daemon.retry_error_interval_s);
            else if (key == "max_line_bytes") parse_int(val, cfg.daemon.max_line_bytes);
            break;
        case Section::none:
        default:
            break;
        }
    }

    return cfg;
}
