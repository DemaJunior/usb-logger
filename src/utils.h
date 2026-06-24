#pragma once

#include <string>
#include <vector>

namespace utils {

// Splits `str` by `delimiter` and returns the parts.
// Empty tokens are preserved (e.g. "a,,b" -> {"a", "", "b"}).
inline std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for (char c : str) {
        if (c == delimiter) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += c;
        }
    }
    tokens.push_back(token); // last segment (may be empty)
    return tokens;
}

} // namespace utils
