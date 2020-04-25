#include "rom/nrom.h"

#include <cassert>
#include <stdexcept>

namespace n_e_s::core {

Nrom::Nrom(const INesHeader &h,
        std::vector<uint8_t> prg_rom,
        std::vector<uint8_t> chr_rom)
        : IRom(h),
          prg_rom_(std::move(prg_rom)),
          chr_rom_(std::move(chr_rom)),
          prg_ram_(static_cast<size_t>(h.prg_ram_size * 8 * 1024)) {
    if (prg_rom_.size() != 16 * 1024 && prg_rom_.size() != 32 * 1024) {
        throw std::invalid_argument("Invalid prg_rom size");
    }

    if (chr_rom_.size() != 8 * 1024) {
        throw std::invalid_argument("Invalid chr_rom size");
    }
}

bool Nrom::is_cpu_address_in_range(uint16_t addr) const {
    const bool in_prg = addr >= kPrgRamStart;
    return in_prg;
}

uint8_t Nrom::cpu_read_byte(uint16_t addr) const {
    if (addr <= kPrgRamEnd) {
        addr -= kPrgRamStart;
        return prg_ram_.at(addr);
    }

    addr -= kPrgRomStart;
    return prg_rom_.at(addr);
}

void Nrom::cpu_write_byte(uint16_t addr, uint8_t byte) {
    if (addr <= kPrgRamEnd) {
        addr -= kPrgRamStart;
        prg_ram_.at(addr) = byte;
    }

    addr -= kPrgRomStart;
    prg_rom_.at(addr) = byte;
}

bool Nrom::is_ppu_address_in_range(uint16_t addr) const {
    const bool in_chr = addr <= kChrEnd;
    return in_chr;
}

uint8_t Nrom::ppu_read_byte(uint16_t addr) const {
    return chr_rom_.at(addr);
}

void Nrom::ppu_write_byte(uint16_t addr, uint8_t byte) {
    chr_rom_.at(addr) = byte;
}

} // namespace n_e_s::core
