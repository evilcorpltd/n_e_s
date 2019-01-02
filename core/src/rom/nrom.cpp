#include "rom/nrom.h"

#include <cassert>

namespace n_e_s::core {

Nrom::Nrom(const INesHeader &h,
        std::vector<uint8_t> prg_rom,
        std::vector<uint8_t> chr_rom)
        : IRom(h),
          prg_rom_(std::move(prg_rom)),
          chr_rom_(std::move(chr_rom)),
          prg_ram_(h.prg_ram_size * 8 * 1024) {
    assert(prg_rom_.size() == 16 * 1024 || prg_rom_.size() == 32 * 1024);
    assert(chr_rom_.size() == 8 * 1024);
}

uint8_t Nrom::read_byte(uint16_t addr) const {
    assert(addr >= prg_ram_start_);

    const std::vector<uint8_t> &memory = translate_address(addr);
    addr -= prg_ram_start_;

    return memory[addr % memory.size()];
}

void Nrom::write_byte(uint16_t addr, uint8_t byte) {
    assert(addr >= prg_ram_start_);

    std::vector<uint8_t> &memory = translate_address(addr);
    addr -= prg_ram_start_;

    memory[addr % memory.size()] = byte;
}

std::vector<uint8_t> &Nrom::translate_address(uint16_t addr) {
    return const_cast<std::vector<uint8_t> &>(
            std::as_const(*this).translate_address(addr));
}

const std::vector<uint8_t> &Nrom::translate_address(uint16_t addr) const {
    if (addr <= prg_ram_end_) {
        return prg_ram_;
    }

    return prg_rom_;
}

} // namespace n_e_s::core