#pragma once

#include "core/irom.h"

#include <string>

namespace n_e_s::core {

class RomFactory {
public:
    static IRom *from_file(const std::string &filepath);
};

} // namespace n_e_s::core
