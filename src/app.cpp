#include "app.h"
#include "serial_port.h"
#include "line_assembler.h"
#include "line_logger.h"
#include "terminal_input.h"
#include "message_classifier.h"
#include "csv_writer.h"
#include "app_config.h"

#include <poll.h>
#include <unistd.h>   // STDIN_FILENO
#include <termios.h>
#include <csignal>
#include <atomic>
#include <iostream>
#include <cstring>

static std::atomic<bool> g_running{true};

static void signal_handler(int) {
    g_running = false;
}

// Put terminal in raw mode so we can read stdin char-by-char without
// waiting for Enter (we still echo manually so the user sees what they type).
struct RawTerminal {
    termios original{};
    bool active{false};

    void enable() {
        if (tcgetattr(STDIN_FILENO, &original) != 0) return;
        termios raw = original;
        raw.c_lflag &= ~(ICANON); // disable line buffering
        // Keep ECHO so the user sees what they type
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        active = true;
    }

    void disable() {
        if (active) {
            tcsetattr(STDIN_FILENO, TCSANOW, &original);
            active = false;
        }
    }

    ~RawTerminal() { disable(); }
};

void run_app(const AppConfig& cfg) {
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    // --- Open serial port ---
    SerialPort serial;
    if (!serial.open(cfg.device, cfg.baud_rate)) {
        std::cerr << "[ERROR] Cannot open serial port: " << cfg.device << '\n';
        return;
    }
    std::cout << "[INFO] Opened " << cfg.device
              << " at " << cfg.baud_rate << " baud\n";

    // --- Open CSV writer ---
    CsvWriter csv(cfg.output_file);
    if (!csv.is_open()) {
        std::cerr << "[ERROR] Cannot open output file: " << cfg.output_file << '\n';
        return;
    }
    std::cout << "[INFO] Logging telemetry to " << cfg.output_file << '\n';
    std::cout << "[INFO] Type commands and press Enter to send to device.\n";
    std::cout << "[INFO] Press Ctrl+C to quit.\n\n";

    // --- Terminal raw mode for non-blocking stdin ---
    RawTerminal term;
    term.enable();

    LineAssembler assembler;
    TerminalInput tty_input;

    // poll(2) watches two fds: the serial port and stdin
    pollfd fds[2];
    fds[0].fd     = serial.fd();   // serial port file descriptor
    fds[0].events = POLLIN;
    fds[1].fd     = STDIN_FILENO;
    fds[1].events = POLLIN;

    while (g_running) {
        int ret = poll(fds, 2, /*timeout_ms=*/100);
        if (ret < 0) {
            if (errno == EINTR) continue; // interrupted by signal
            std::cerr << "[ERROR] poll() failed: " << strerror(errno) << '\n';
            break;
        }

        // ---- Data available from the device ----
        if (fds[0].revents & POLLIN) {
            char buf[256];
            ssize_t n = read(serial.fd(), buf, sizeof(buf));
            if (n <= 0) {
                std::cerr << "[WARN] Serial port closed or read error.\n";
                break;
            }
            for (ssize_t i = 0; i < n; ++i) {
                std::string line;
                if (assembler.push(buf[i], line)) {
                    TelemetryRecord rec;
                    if (classify_message(line, rec)) {
                        // Valid telemetry -> save to CSV
                        csv.write(rec);
                        std::cout << "[CSV] " << line << '\n';
                    } else {
                        // Anything else -> just print
                        std::cout << "[DEV] " << line << '\n';
                    }
                }
            }
        }

        // ---- User typed something ----
        if (fds[1].revents & POLLIN) {
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                std::string cmd;
                if (tty_input.push(c, cmd)) {
                    // Send the command (with newline) to the device
                    cmd += '\n';
                    ssize_t written = write(serial.fd(), cmd.data(), cmd.size());
                    if (written < 0) {
                        std::cerr << "[ERROR] Failed to write to serial: "
                                  << strerror(errno) << '\n';
                    }
                    // Re-print so the sent command is clearly visible
                    std::cout << "[TX] " << cmd;
                }
            }
        }
    }

    std::cout << "\n[INFO] Exiting.\n";
    term.disable();
    serial.close();
}
