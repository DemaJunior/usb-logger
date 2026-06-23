#include "terminal_input.h"

bool TerminalInput::push(char c, std::string& out) {
    if (c == '\n' || c == '\r') {
        if (!m_buf.empty()) {
            out = m_buf;
            m_buf.clear();
            return true;
        }
        return false;
    }
    m_buf += c;
    return false;
}
