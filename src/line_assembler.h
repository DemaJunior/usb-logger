#pragma once

#include <string>
#include <vector>

class LineAssembler {
public:
    explicit LineAssembler(std::size_t maxLineBytes);

    // Feed raw bytes; returns completed lines (without '\n'/'\r')
    std::vector<std::string> feed(const std::string& chunk);

private:
    std::size_t max_;
    std::string buf_;
};
