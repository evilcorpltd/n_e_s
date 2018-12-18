#include "ppu.h"

#include <stdexcept>

namespace n_e_s::core {

Ppu::Ppu(std::unique_ptr<PpuRegisters> registers)
        : registers_(std::move(registers)) {
}

uint8_t Ppu::read_byte(uint16_t addr) {
    if (addr == 0x2002) {
        const uint8_t status = registers_->status;
        clear_vblank_flag();
        return status;
    }

    throw std::invalid_argument("Address not supported");
}

void Ppu::write_byte(uint16_t, uint8_t) {}

void Ppu::execute() {}

void Ppu::clear_vblank_flag() {
    registers_->status &= ~(1 << 7);
}

} // namespace n_e_s::core
