#include "rom/mapper_2.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include "nes/core/ines_header.h"
#include "nes/core/invalid_address.h"

namespace n_e_s::core {

Mapper2::Mapper2(const INesHeader &h,
        std::vector<uint8_t> prg_rom,
        std::vector<uint8_t> chr_mem)
        : IRom(h),
          select_bank_hi_(h.prg_rom_size - 1),
          prg_rom_(std::move(prg_rom)),
          chr_mem_(std::move(chr_mem)),
          nametables_{} {
    if (prg_rom_.size() !=
            static_cast<std::size_t>(16u * 1024u * h.prg_rom_size)) {
        throw std::invalid_argument("Invalid prg_rom size");
    }

    if (chr_mem_.size() != static_cast<std::size_t>(8u * 1024u)) {
        throw std::invalid_argument("Invalid chr_ram size");
    }
}

bool Mapper2::is_cpu_address_in_range(uint16_t addr) const {
    const bool in_prg = addr >= 0x8000;
    return in_prg;
}

uint8_t Mapper2::cpu_read_byte(uint16_t addr) const {
    if (addr >= kSwitchablePrgRomStart && addr <= kSwitchablePrgRomEnd) {
        const uint32_t mapped_addr =
                select_bank_low_ * 0x4000u + (addr & 0x3FFFu);
        return prg_rom_[mapped_addr];
    }

    if (addr >= kLastBankPrgRomStart) {
        const uint32_t mapped_addr =
                select_bank_hi_ * 0x4000u + (addr & 0x3FFFu);
        return prg_rom_[mapped_addr];
    }

    throw InvalidAddress(addr);
}

void Mapper2::cpu_write_byte(uint16_t addr, uint8_t byte) {
    if (addr >= kSwitchablePrgRomStart) {
        select_bank_low_ = byte & 0x0Fu;
    }
}

bool Mapper2::is_ppu_address_in_range(uint16_t addr) const {
    const bool in_chr = addr <= kChrEnd;
    const bool in_nametable = addr >= kNametableStart && addr <= kNametableEnd;
    return in_chr || in_nametable;
}

uint8_t Mapper2::ppu_read_byte(uint16_t addr) const {
    if (addr <= kChrEnd) {
        return chr_mem_.at(addr);
    }
    const auto [index, addr_mod] =
            translate_nametable_addr(addr, header().mirroring());
    return nametables_[index][addr_mod];
}

void Mapper2::ppu_write_byte(uint16_t addr, uint8_t byte) {
    if (addr <= kChrEnd) {
        chr_mem_.at(addr) = byte;
    }
    const auto [index, addr_mod] =
            translate_nametable_addr(addr, header().mirroring());
    nametables_[index][addr_mod] = byte;
}

std::pair<int, uint16_t> Mapper2::translate_nametable_addr(uint16_t addr,
        Mirroring m) const {
    // TODO(johnor): This logic is identical to mapper 0 (Nrom).
    // Refactor and move outside the mapper.

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
