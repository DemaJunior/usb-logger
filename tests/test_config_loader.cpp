#include "test_framework.h"

#include <filesystem>
#include <fstream>

#include "config_loader.h"

static std::string write_temp(const std::string& content) {
    auto dir = std::filesystem::temp_directory_path();
    auto path = dir / "usb_logger_test_config.toml";
    std::ofstream out(path);
    out << content;
    out.close();
    return path.string();
}

TEST_CASE("ConfigLoader parses serial and daemon fields") {
    const auto path = write_temp(R"(
[serial]
port = "/dev/ttyTEST0"
baudrate = 57600
parity = "even"
databits = 7
stopbits = 2
read_timeout_ms = 250

[daemon]
retry_port_interval_s = 3
retry_error_interval_s = 4
max_line_bytes = 123

[log]
directory = "/tmp"
base_name = "u"
reopen_interval_s = 1
flush_each_line = false
)" );

    auto cfgOpt = ConfigLoader::load(path);
    REQUIRE(cfgOpt.has_value());

    const auto& cfg = *cfgOpt;
    REQUIRE(cfg.serial.port == "/dev/ttyTEST0");
    REQUIRE(cfg.serial.baudrate == 57600);
    REQUIRE(cfg.serial.parity == "even");
    REQUIRE(cfg.serial.databits == 7);
    REQUIRE(cfg.serial.stopbits == 2);
    REQUIRE(cfg.serial.read_timeout_ms == 250);

    REQUIRE(cfg.daemon.retry_port_interval_s == 3);
    REQUIRE(cfg.daemon.retry_error_interval_s == 4);
    REQUIRE(cfg.daemon.max_line_bytes == 123);

    REQUIRE(cfg.log.directory == "/tmp");
    REQUIRE(cfg.log.base_name == "u");
    REQUIRE(cfg.log.reopen_interval_s == 1);
    REQUIRE(cfg.log.flush_each_line == false);
}

TEST_CASE("ConfigLoader returns nullopt when file missing") {
    auto cfgOpt = ConfigLoader::load("/path/that/does/not/exist.toml");
    REQUIRE(!cfgOpt.has_value());
}
