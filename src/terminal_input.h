#pragma once
#include <string>
#include <functional>

// Reads a line from stdin without blocking the main loop.
// Feed it a character at a time (from select/poll on STDIN_FILENO).
class TerminalInput {
public:
    // Returns true and sets `out` when a full line (\n-terminated) is ready.
    bool push(char c, std::string& out);
    void clear() { m_buf.clear(); }
private:
    std::string m_buf;
};
