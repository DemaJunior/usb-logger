#pragma once

#include <cstdint>
#include <string>

struct SerialConfig {
    std::string port{ "/dev/ttyUSB0" };
    int baudrate{ 115200 };
    std::string parity{ "none" };
    int databits{ 8 };
    int stopbits{ 1 };
    int read_timeout_ms{ 500 };
};

struct LogConfig {
    std::string directory{ "/var/log/usb-logger" };
    std::string base_name{ "usb-logger" };
    int reopen_interval_s{ 5 };
    bool flush_each_line{ true };
};

struct DaemonConfig {
    int retry_port_interval_s{ 2 };
    int retry_error_interval_s{ 2 };
    int max_line_bytes{ 4096 };
};

struct AppConfig {
    SerialConfig serial{};
    LogConfig log{};
    DaemonConfig daemon{};
    std::string config_path{};
};
