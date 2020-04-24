#pragma once

#include "nes/core/irom.h"

#include <iosfwd>
#include <memory>
#include <string>

namespace n_e_s::core {

class RomFactory {
public:
    [[nodiscard]] static std::unique_ptr<IRom> from_file(
            const std::string &filepath);
    [[nodiscard]] static std::unique_ptr<IRom> from_bytes(
            std::istream &bytestream);
};

} // namespace n_e_s::core
