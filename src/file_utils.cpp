#include "file_utils.h"

#include <filesystem>

bool file_utils::ensure_directory(const std::string& path) {
    std::error_code ec;
    if (path.empty()) return false;
    std::filesystem::create_directories(path, ec);
    if (ec) return false;
    return std::filesystem::is_directory(path);
}
