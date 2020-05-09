#pragma once

#include <cstdint>

namespace n_e_s::core::test {

constexpr uint8_t u16_to_u8(uint16_t u16) {
    return static_cast<uint8_t>(u16 % static_cast<uint16_t>(0x100));
}

} // namespace n_e_s::core::test
