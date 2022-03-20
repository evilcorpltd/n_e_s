#include "rom/nrom.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include "nes/core/ines_header.h"

namespace n_e_s::core {

Nrom::Nrom(const INesHeader &h,
        std::vector<uint8_t> prg_rom,
        std::vector<uint8_t> chr_rom)
        : IRom(h),
          prg_rom_(std::move(prg_rom)),
          chr_rom_(std::move(chr_rom)),
          prg_ram_(static_cast<size_t>(h.prg_ram_size * 8 * 1024)),
          nametables_{} {
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
        return prg_ram_[addr % prg_ram_.size()];
    }

    addr -= kPrgRomStart;
    return prg_rom_[addr % prg_rom_.size()];
}

void Nrom::cpu_write_byte(uint16_t addr, uint8_t byte) {
    // Only ram is writable
    if (addr <= kPrgRamEnd) {
        addr -= kPrgRamStart;
        prg_ram_[addr % prg_ram_.size()] = byte;
    }
}

bool Nrom::is_ppu_address_in_range(uint16_t addr) const {
    const bool in_chr = addr <= kChrEnd;
    const bool in_nametable = addr >= kNametableStart && addr <= kNametableEnd;
    return in_chr || in_nametable;
}

uint8_t Nrom::ppu_read_byte(uint16_t addr) const {
    if (addr <= kChrEnd) {
        return chr_rom_.at(addr);
    }
    const auto [index, addr_mod] =
            translate_nametable_addr(addr, header().mirroring());
    return nametables_[index][addr_mod];
}

void Nrom::ppu_write_byte(uint16_t addr, uint8_t byte) {
    if (addr <= kChrEnd) {
        chr_rom_.at(addr) = byte;
    }
    const auto [index, addr_mod] =
            translate_nametable_addr(addr, header().mirroring());
    nametables_[index][addr_mod] = byte;
}

std::pair<int, uint16_t> Nrom::translate_nametable_addr(uint16_t addr,
        Mirroring m) const {
    // Nametables
    // Range        Size    Desc
    // $2000-$23FF  $0400   Nametable 0
    // $2400-$27FF  $0400   Nametable 1
    // $2800-$2BFF  $0400   Nametable 2
    // $2C00-$2FFF  $0400   Nametable 3
    // $3000-$3EFF  $0F00   Mirrors of $2000-$2EFF

    // Ignore top 4 bits to handle mirroring
    addr &= 0x0FFFu;
    if (m == Mirroring::Horizontal) {
        // Nametable 0 and 1 should be the same
        if (addr <= 0x03FF) {
            return {0, addr % 0x0400};
        }
        if (addr >= 0x0400 && addr <= 0x07FF) {
            return {0, addr % 0x0400};
        }
        if (addr >= 0x0800 && addr <= 0x0BFF) {
            return {1, addr % 0x0400};
        }
        if (addr >= 0x0C00 && addr <= 0x0FFF) {
            return {1, addr % 0x0400};
        }
    } else if (header().mirroring() == Mirroring::Vertical) {
        // Nametable 0 and 3 should be the same
        if (addr <= 0x03FF) {
            return {0, addr % 0x0400};
        }
        if (addr >= 0x0400 && addr <= 0x07FF) {
            return {1, addr % 0x0400};
        }
        if (addr >= 0x0800 && addr <= 0x0BFF) {
            return {0, addr % 0x0400};
        }
        if (addr >= 0x0C00 && addr <= 0x0FFF) {
            return {1, addr % 0x0400};
        }
    }
    throw std::runtime_error("Unknown address: " + std::to_string(addr));
}

} // namespace n_e_s::core
