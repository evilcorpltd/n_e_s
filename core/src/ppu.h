#pragma once

#include "core/ippu.h"

namespace n_e_s::core {

class Ppu : public IPpu {
public:
    Ppu() = default;

    uint8_t read_byte(uint16_t addr) const override;
    void write_byte(uint16_t addr, uint8_t byte) override;

    void execute() override;
};

} // namespace n_e_s::core
