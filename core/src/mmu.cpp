// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mmu.h"

#include <algorithm>

namespace n_e_s::core {

Mmu::Mmu() : mem_banks_() {}

void Mmu::add_mem_bank(IMemBank *mem_bank) {
    mem_banks_.push_back(mem_bank);
}

IMemBank *Mmu::get_mem_bank(uint16_t addr) const {
    auto it = std::find_if(
            begin(mem_banks_), end(mem_banks_), [=](IMemBank *mem_bank) {
                return mem_bank->is_address_in_range(addr);
            });

    if (it != end(mem_banks_)) {
        return *it;
    } else {
        return nullptr;
    }
}

uint8_t Mmu::read_byte(uint16_t addr) const {
    IMemBank *mem_bank = get_mem_bank(addr);
    return mem_bank ? mem_bank->read_byte(addr) : 0;
}

uint16_t Mmu::read_word(uint16_t addr) const {
    IMemBank *mem_bank = get_mem_bank(addr);
    return mem_bank ? mem_bank->read_word(addr) : 0;
}

void Mmu::write_byte(uint16_t addr, uint8_t byte) {
    IMemBank *mem_bank = get_mem_bank(addr);
    if (mem_bank) {
        mem_bank->write_byte(addr, byte);
    }
}

void Mmu::write_word(uint16_t addr, uint16_t word) {
    IMemBank *mem_bank = get_mem_bank(addr);
    if (mem_bank) {
        mem_bank->write_word(addr, word);
    }
}

} // namespace n_e_s::core
