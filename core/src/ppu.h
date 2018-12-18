#pragma once

#include "core/ippu.h"

#include <memory>

namespace n_e_s::core {

class Ppu : public IPpu {
public:
    Ppu(std::unique_ptr<PpuRegisters> registers);

    uint8_t read_byte(uint16_t addr) override;
    void write_byte(uint16_t addr, uint8_t byte) override;

    void execute() override;

private:
    std::unique_ptr<PpuRegisters> registers_;

    void clear_vblank_flag();
};

} // namespace n_e_s::core
