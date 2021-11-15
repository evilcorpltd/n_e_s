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
};

} // namespace n_e_s::core
