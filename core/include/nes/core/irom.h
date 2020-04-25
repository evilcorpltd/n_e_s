#pragma once

#include "nes/core/ines_header.h"

namespace n_e_s::core {

class IRom {
public:
    explicit IRom(const INesHeader &h) : header_(h) {}

    virtual ~IRom() = default;

    [[nodiscard]] virtual bool is_cpu_address_in_range(uint16_t addr) const = 0;
    [[nodiscard]] virtual uint8_t cpu_read_byte(uint16_t addr) const = 0;
    virtual void cpu_write_byte(uint16_t addr, uint8_t byte) = 0;

    [[nodiscard]] virtual bool is_ppu_address_in_range(uint16_t addr) const = 0;
    [[nodiscard]] virtual uint8_t ppu_read_byte(uint16_t addr) const = 0;
    virtual void ppu_write_byte(uint16_t addr, uint8_t byte) = 0;

private:
    INesHeader header_;
};

} // namespace n_e_s::core
