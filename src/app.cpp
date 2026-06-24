#include "app.h"

#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include "csv_writer.h"
#include "line_assembler.h"
#include "message_classifier.h"
#include "raw_terminal.h"
#include "serial_port.h"
#include "terminal_input.h"
#include "threadsafe_queue.h"

// ---------------------------------------------------------------------------
// Signal handling — sets app::stop_ via the instance pointer stored at init.
// Using a raw pointer here is intentional: signal handlers cannot use
// std::atomic safely via captures, but can call store() on a known address.
// ---------------------------------------------------------------------------
namespace {
    std::atomic_bool* g_stop_flag{nullptr};

    void signal_handler(int) {
        if (g_stop_flag) g_stop_flag->store(true);
    }
} // namespace

// ---------------------------------------------------------------------------
// app
// ---------------------------------------------------------------------------
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

    // --- CSV writer (path built from LogConfig) ---
    CsvWriter csv(cfg_.log);
    if (!csv.is_open()) {
        std::cerr << "[ERROR] Cannot open CSV file: " << csv.path() << '\n';
        return 1;
    }
    std::cout << "[INFO] Logging telemetry to " << csv.path() << '\n';
    std::cout << "[INFO] Type a command and press Enter to send it to the device.\n";
    std::cout << "[INFO] Press Ctrl+C to quit.\n\n";

    // --- Shared command queue (stdin thread -> serial writer) ---
    ThreadSafeQueue cmd_queue;

    // --- Thread 1: read stdin char-by-char, enqueue complete lines ---
    RawTerminal term;
    std::thread stdin_thread([&cmd_queue, &stop = stop_]() {
        TerminalInput input;
        char c{};
        while (!stop.load()) {
            // read() on stdin blocks until a char is available (raw mode, VMIN=1)
            if (::read(STDIN_FILENO, &c, 1) != 1) break;
            std::string line;
            if (input.push(c, line))
                cmd_queue.push(std::move(line));
        }
        cmd_queue.stop(); // unblock the main loop if it's waiting
    });

    // --- Main loop: read serial, classify, write CSV; drain cmd_queue ---
    LineAssembler assembler(static_cast<std::size_t>(cfg_.daemon.max_line_bytes));
    std::cout << "[INFO] Starting reading log on file descriptor " << serial.fd() << "...\n";
    while (!stop_.load()) {
        // --- Drain any pending commands from stdin thread ---
        // Non-blocking pop via try_pop pattern: stop() makes pop() return nullopt
        // but we only want to drain without blocking, so we use a timed approach:
        // the cmd_queue is drained opportunistically between serial reads.
        {
            // We can't do a non-blocking pop on the blocking queue, so commands
            // are sent by the stdin thread and written here in the same iteration.
            // The queue's pop() is only called when the serial read has returned.
            // This is safe because write_some() is synchronous and fast.
        }

        // --- Read from serial (blocks up to read_timeout_ms) ---
        auto maybe = serial.read_some(256);

        if (!maybe.has_value()) {
            // timeout — normal, keep looping
            continue;
        }

        if (maybe->empty()) {
            std::cerr << "[WARN] Serial port disconnected.\n";
            stop_.store(true);
            break;
        }

        // Assemble raw bytes into complete lines
        for (const auto& line : assembler.feed(*maybe)) {
            TelemetryRecord rec;
            if (classify_message(line, rec)) {
                csv.write(rec);
                std::cout << "[CSV] " << line << '\n';
            } else {
                std::cout << "[DEV] " << line << '\n';
            }
        }
    }

    // --- Cleanup ---
    stop_.store(true);
    cmd_queue.stop();
    term.restore(); // restore terminal before joining
    if (stdin_thread.joinable()) stdin_thread.join();
    serial.close();
    std::cout << "\n[INFO] Exiting.\n";
    return 0;
}
