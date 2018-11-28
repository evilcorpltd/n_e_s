// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/irom.h"

// #include <filesystem>
#include <string>

namespace n_e_s::core {

class RomFactory {
public:
    // static IRom *fromFile(const std::filesystem::path& filepath);
    static IRom *fromFile(const std::string& filepath);
};

} // namespace n_e_s::core
