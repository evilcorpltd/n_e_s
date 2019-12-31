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

// The cartridge owns all space up to 0x1FFF (Chr) and 0x6000 to 0xFFFF (Prg).
bool Nrom::is_address_in_range(uint16_t addr) const {
    const bool in_chr = addr <= kChrEnd;
    const bool in_prg = addr >= kPrgRamStart;
    return in_chr || in_prg;
}

uint8_t Nrom::read_byte(uint16_t addr) const {
    assert(addr <= kChrEnd || addr >= kPrgRamStart);

    const std::vector<uint8_t> &memory = translate_address(addr);
    // Adjust address if we are reading from Prg
    if (addr >= kPrgRamStart) {
        addr -= addr <= kPrgRamEnd ? kPrgRamStart : kPrgRomStart;
    }

    return memory[addr % memory.size()];
}

void Nrom::write_byte(uint16_t addr, uint8_t byte) {
    assert(addr <= kChrEnd || addr >= kPrgRamStart);

    std::vector<uint8_t> &memory = translate_address(addr);
    // Adjust address if we are reading from Prg
    if (addr >= kPrgRamStart) {
        addr -= addr <= kPrgRamEnd ? kPrgRamStart : kPrgRomStart;
    }

    memory[addr % memory.size()] = byte;
}

std::vector<uint8_t> &Nrom::translate_address(uint16_t addr) {
    return const_cast<std::vector<uint8_t> &>(
            std::as_const(*this).translate_address(addr));
}

const std::vector<uint8_t> &Nrom::translate_address(uint16_t addr) const {
    if (addr <= kChrEnd) {
        return chr_rom_;
    }
    if (addr <= kPrgRamEnd) {
        return prg_ram_;
    }

    return prg_rom_;
}

} // namespace n_e_s::core
