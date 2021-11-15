#pragma once

#include <cstdint>
#include <functional>

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

class IPpu {
public:
    virtual ~IPpu() = default;

    virtual uint8_t read_byte(uint16_t addr) = 0;
    virtual void write_byte(uint16_t addr, uint8_t byte) = 0;

    virtual void execute() = 0;

    virtual void set_nmi_handler(const std::function<void()> &nmi_handler) = 0;

    [[nodiscard]] virtual uint16_t scanline() const = 0;
    [[nodiscard]] virtual uint16_t cycle() const = 0;
};

} // namespace n_e_s::core
