// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mmu.h"

namespace n_e_s::core {

Mmu::Mmu() : memory_() {
}

uint8_t Mmu::read_byte(uint16_t addr) const {
    return memory_[addr];
}

uint16_t Mmu::read_word(uint16_t addr) const {
    return memory_[addr] | memory_[addr + 1] << 8;
}

void Mmu::write_byte(uint16_t addr, uint8_t byte) {
    memory_[addr] = byte;
}

void Mmu::write_word(uint16_t addr, uint16_t word) {
    memory_[addr] = word & 0xFF;
    memory_[addr + 1] = word >> 8;
}

}
