#include "nes/core/invalid_address.h"

#include <fmt/format.h>

namespace n_e_s::core {

InvalidAddress::InvalidAddress(uint16_t addr)
        : message_(fmt::format("Invalid address [0x{:04X}]", addr)) {}

const char *InvalidAddress::what() const noexcept {
    return message_.c_str();
}

} // namespace n_e_s::core
