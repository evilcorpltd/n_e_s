#include "mapped_cpu_register.h"

namespace n_e_s::core {

namespace {

constexpr uint16_t kOamDma = 0x4014;
constexpr uint16_t kOamData = 0x2004;

} // namespace

MappedCpuRegister::MappedCpuRegister(IPpu *ppu, IMemBank *cpu_ram)
        : ppu_(ppu), cpu_ram_(cpu_ram) {}

bool MappedCpuRegister::is_address_in_range(uint16_t addr) const {
    return addr >= 0x4000 && addr <= 0x4017;
}

uint8_t MappedCpuRegister::read_byte([[maybe_unused]] uint16_t addr) const {
    // TODO(JN) Which registers can be read from? What should we return
    // otherwise?
    return 0u;
}
void MappedCpuRegister::write_byte(uint16_t addr, uint8_t byte) {
    if (addr == kOamDma) {
        do_dma(byte);
    }
}

void MappedCpuRegister::do_dma(uint8_t byte) {
    for (uint16_t i = 0; i < 256u; i++) {
        const uint8_t b = cpu_ram_->read_byte(byte * 0x0100 + i);
        ppu_->write_byte(kOamData, b);
    }
}

} // namespace n_e_s::core
