#pragma once

#include <cstdint>

namespace n_e_s::core {

enum class Mirroring { Horizontal, Vertical };

struct INesHeader {
    uint8_t nes[4]{'N', 'E', 'S', 26}; // NES + MS-DOS EOF
    uint8_t prg_rom_size; // in 16 KB units.
    uint8_t chr_rom_size; // in 8 KB units.
    uint8_t flags_6;
    uint8_t flags_7;
    uint8_t prg_ram_size; // in 8 KB units.
    uint8_t flags_9;
    uint8_t flags_10;
    uint8_t zeros[5]{0};

    [[nodiscard]] constexpr uint8_t mapper() const {
        uint8_t mapper = static_cast<uint8_t>(flags_6 & 0xF0u) >> 4u;
        mapper |= flags_7 & 0xF0u;
        return mapper;
    }

    [[nodiscard]] constexpr Mirroring mirroring() const {
        return flags_6 & 0x01u ? Mirroring::Vertical : Mirroring::Horizontal;
    }
};

} // namespace n_e_s::core
