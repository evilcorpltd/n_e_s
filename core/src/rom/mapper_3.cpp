#include "rom/mapper_3.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#include "nes/core/ines_header.h"
#include "nes/core/invalid_address.h"

namespace n_e_s::core {

Mapper3::Mapper3(const INesHeader &h,
        std::vector<uint8_t> prg_rom,
        std::vector<uint8_t> chr_mem)
        : IRom(h), prg_rom_(std::move(prg_rom)), chr_mem_(std::move(chr_mem)) {
    prg_rom_size_ = h.prg_rom_size;

    if (prg_rom_.size() !=
            static_cast<std::size_t>(16u * 1024u * h.prg_rom_size)) {
        throw std::invalid_argument("Invalid prg_rom size");
    }

    if (chr_mem_.size() !=
            static_cast<std::size_t>(8u * 1024u * h.chr_rom_size)) {
        throw std::invalid_argument("Invalid chr_ram size");
    }
}

bool Mapper3::is_cpu_address_in_range(uint16_t addr) const {
    const bool in_prg = addr >= kPrgRomStart;
    return in_prg;
}

uint8_t Mapper3::cpu_read_byte(uint16_t addr) const {
    if (addr >= kPrgRomStart) {
        uint16_t mapped_addr = addr - kPrgRomStart;
        if (prg_rom_size_ == 1) {
            // Mirroring
            mapped_addr = mapped_addr & 0x3FFFu;
        }
        return prg_rom_[mapped_addr];
    }
    throw InvalidAddress(addr);
}

void Mapper3::cpu_write_byte(uint16_t addr, uint8_t byte) {
    (void)addr;
    n_chr_bank_select_ = byte & 0x03u;
}

bool Mapper3::is_ppu_address_in_range(uint16_t addr) const {
    const bool in_chr = addr < kChrWindow;
    const bool in_nametable = addr >= kNametableStart && addr <= kNametableEnd;
    return in_chr || in_nametable;
}

uint8_t Mapper3::ppu_read_byte(uint16_t addr) const {
    if (addr < kChrWindow) {
        const uint32_t mapped_addr = n_chr_bank_select_ * kChrWindow + addr;
        return chr_mem_.at(mapped_addr);
    }

    const auto [index, addr_mod] =
            translate_nametable_addr(addr, header().mirroring());
    return nametables_[index][addr_mod];
}

void Mapper3::ppu_write_byte(uint16_t addr, uint8_t byte) {
    if (addr < kChrWindow) {
        const uint32_t mapped_addr = n_chr_bank_select_ * kChrWindow + addr;
        chr_mem_.at(mapped_addr) = byte;
    } else {
        const auto [index, addr_mod] =
                translate_nametable_addr(addr, header().mirroring());
        nametables_[index][addr_mod] = byte;
    }
}

std::pair<int, uint16_t> Mapper3::translate_nametable_addr(uint16_t addr,
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
