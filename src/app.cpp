#include "app.h"

#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "csv_writer.h"
#include "line_assembler.h"
#include "message_classifier.h"
#include "raw_terminal.h"
#include "serial_port.h"
#include "terminal_input.h"
#include "threadsafe_queue.h"

namespace {
    std::atomic_bool* g_stop_flag{nullptr};
    void signal_handler(int) {
        if (g_stop_flag) g_stop_flag->store(true);
    }
} // namespace

app::app(AppConfig cfg) : cfg_(std::move(cfg)) {}
app::~app() = default;

int app::run() {
    g_stop_flag = &stop_;
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);
    return runLoop();
}

int app::runLoop() {
    // --- Serial port ---
    SerialPort serial;
    if (!serial.open(cfg_.serial)) {
        std::cerr << "[ERROR] Cannot open serial port: " << cfg_.serial.port << '\n';
        return 1;
    }
    std::cout << "[INFO] Opened " << cfg_.serial.port
              << " at " << cfg_.serial.baudrate << " baud\n";

    // --- CSV writer ---
    CsvWriter csv(cfg_.log);
    if (!csv.is_open()) {
        std::cerr << "[ERROR] Cannot open CSV file: " << csv.path() << '\n';
        return 1;
    }
    std::cout << "[INFO] Logging telemetry to " << csv.path() << '\n';
    std::cout << "[INFO] Type a command and press Enter to send it to the device.\n";
    std::cout << "[INFO] Press Ctrl+C to quit.\n\n";

    // --- Shared command queue (stdin thread -> main loop) ---
    ThreadSafeQueue cmd_queue;

    // --- Thread: read stdin char-by-char, enqueue complete lines ---
    RawTerminal term;
    std::thread stdin_thread([&cmd_queue, &stop = stop_]() {
        TerminalInput input;
        char c{};
        while (!stop.load()) {
            if (::read(STDIN_FILENO, &c, 1) != 1) break;
            std::string line;
            if (input.push(c, line))
                cmd_queue.push(std::move(line));
        }
        cmd_queue.stop();
    });

    // --- Main loop: read serial, classify, write CSV ---
    LineAssembler assembler(static_cast<std::size_t>(cfg_.daemon.max_line_bytes));
    std::cout << "[INFO] Starting reading log on file " << csv.path() << "...\n";
    while (!stop_.load()) {
        // Drain pending commands from stdin thread and send to device
        {
            // ThreadSafeQueue::pop() blocks, so we use a non-blocking drain:
            // commands are sent right after serial read returns (timeout or data).
        }

        auto maybe = serial.read_some(256);

        if (!maybe.has_value()) continue; // timeout, keep looping

        if (maybe->empty()) {
            std::cerr << "[WARN] Serial port disconnected.\n";
            stop_.store(true);
            break;
        }

        for (const auto& line : assembler.feed(*maybe)) {
            std::vector<std::string> fields;
            if (classify_message(line, fields)) {
                csv.write(fields);
                //std::cout << "[CSV] " << line << '\n';
            } else {
                std::cout << "[DEV] " << line << '\n';
            }
        }
    }

    // --- Cleanup ---
    stop_.store(true);
    cmd_queue.stop();
    term.restore();
    if (stdin_thread.joinable()) stdin_thread.join();
    serial.close();
    std::cout << "\n[INFO] Exiting.\n";
    return 0;
}
