#pragma once

#include <cstdint>

namespace n_e_s::core {

struct PpuRegisters {
    uint16_t scanline;
    uint16_t cycle;
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oamaddr;
    uint8_t fine_x_scroll;
    uint16_t vram_addr;
    uint16_t temp_vram_addr;
    bool write_toggle;

    uint8_t name_table_latch;
    uint8_t name_table;

    uint8_t pattern_table_latch_low;
    uint8_t pattern_table_latch_hi;
    uint16_t pattern_table_shifter_low;
    uint16_t pattern_table_shifter_hi;

    uint8_t attribute_table_latch;
    uint16_t attribute_table_shifter_low;
    uint16_t attribute_table_shifter_hi;

    [[nodiscard]] constexpr bool is_rendering_enabled() const {
        return (mask & (1u << 3u)) || (mask & (1u << 4u));
    }
};

} // namespace n_e_s::core
