#pragma once
// RAII wrapper that disables canonical (line-buffered) mode on stdin
// so characters are delivered immediately without waiting for Enter.
// ECHO is kept enabled so the user can see what they type.
// Restores the original terminal settings on destruction.

#include <termios.h>
#include <unistd.h>

class RawTerminal {
public:
    RawTerminal() {
        if (tcgetattr(STDIN_FILENO, &original_) != 0) return;
        termios raw = original_;
        raw.c_lflag &= ~static_cast<tcflag_t>(ICANON); // char-by-char, keep ECHO
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0)
            active_ = true;
    }

    ~RawTerminal() { restore(); }

    // Non-copyable, non-movable — owns terminal state
    RawTerminal(const RawTerminal&) = delete;
    RawTerminal& operator=(const RawTerminal&) = delete;

    void restore() {
        if (active_) {
            tcsetattr(STDIN_FILENO, TCSANOW, &original_);
            active_ = false;
        }
    }

private:
    termios original_{};
    bool    active_{false};
};
