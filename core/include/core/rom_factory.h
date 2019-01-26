#pragma once

#include "core/irom.h"

#include <iosfwd>
#include <memory>
#include <string>

namespace n_e_s::core {

class RomFactory {
public:
    static std::unique_ptr<IRom> from_file(const std::string &filepath);
    static std::unique_ptr<IRom> from_bytes(std::istream &bytestream);
};

} // namespace n_e_s::core
