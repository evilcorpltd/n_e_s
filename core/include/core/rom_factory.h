#pragma once

#include "core/irom.h"

#include <istream>
#include <string>

namespace n_e_s::core {

class RomFactory {
public:
    static IRom *from_file(const std::string &filepath);
    static IRom *from_bytes(std::istream &bytestream);
};

} // namespace n_e_s::core
