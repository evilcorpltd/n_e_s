// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mmu.h"

#include <utility>

namespace n_e_s::core {

Mmu::Mmu() : ram_(), ppu_(), io_(), io_dev_(), cartridge_() {
}

uint8_t Mmu::read_byte(uint16_t addr) const {
    return *translate_address(addr);
}

uint16_t Mmu::read_word(uint16_t addr) const {
    return read_byte(addr) | read_byte(addr + 1) << 8;
}

void Mmu::write_byte(uint16_t addr, uint8_t byte) {
    *translate_address(addr) = byte;
}

void Mmu::write_word(uint16_t addr, uint16_t word) {
    write_byte(addr, word & 0xFF);
    write_byte(addr + 1, word >> 8);
}

uint8_t *Mmu::translate_address(uint16_t addr) {
    return const_cast<uint8_t *>(std::as_const(*this).translate_address(addr));
}

const uint8_t *Mmu::translate_address(uint16_t addr) const {
    if (addr <= ram_end_) {
        addr %= ram_size_;
        return &ram_[addr];
    }

    if (addr <= ppu_end_) {
        addr %= ppu_size_;
        return &ppu_[addr];
    }

    if (addr <= io_end_) {
        addr %= io_size_;
        return &io_[addr];
    }

    if (addr <= io_dev_end_) {
        addr %= io_dev_size_;
        return &io_dev_[addr];
    }

    addr %= cartridge_size_;
    return &cartridge_[addr];
}

}
