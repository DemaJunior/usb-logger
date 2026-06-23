#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "app_config.h"

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    bool open(const SerialConfig& cfg);
    void close();

    bool is_open() const;

    // Reads up to `maxBytes`. Returns:
    // - nullopt on timeout (no data)
    // - empty string on EOF / disconnected
    // - string with bytes read
    std::optional<std::string> read_some(std::size_t maxBytes);

    // Writes `data` to the serial port.
    // Returns true if all bytes were written successfully.
    bool write_some(const std::string& data);

private:
    int fd_{-1};
    int read_timeout_ms_{500};
};
