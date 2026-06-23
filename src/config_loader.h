#pragma once

#include <optional>
#include <string>

#include "app_config.h"

class ConfigLoader {
public:
    static std::optional<AppConfig> load(const std::string& path);
};
