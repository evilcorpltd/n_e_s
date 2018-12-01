// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mmu.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace n_e_s::core {

namespace {

auto equal(uint16_t addr) {
    return [=](const auto &mem_bank) {
        return mem_bank->is_address_in_range(addr);
    };
}

std::string invalid_address_msg(uint16_t addr) {
    std::stringstream ss;
    ss << "Invalid address [" << addr << "]";
    return ss.str();
}

} // namespace

Mmu::Mmu() : mem_banks_() {}

void Mmu::add_mem_bank(std::unique_ptr<IMemBank> mem_bank) {
    mem_banks_.push_back(std::move(mem_bank));
}

IMemBank *Mmu::get_mem_bank(uint16_t addr) const {
    auto it = std::find_if(begin(mem_banks_), end(mem_banks_), equal(addr));
    if (it != end(mem_banks_)) {
        return (*it).get();
    } else {
        return nullptr;
    }
}

uint8_t Mmu::read_byte(uint16_t addr) const {
    if (const IMemBank *mem_bank = get_mem_bank(addr)) {
        return mem_bank->read_byte(addr);
    } else {
        throw std::invalid_argument(invalid_address_msg(addr));
    }
}

uint16_t Mmu::read_word(uint16_t addr) const {
    if (const IMemBank *mem_bank = get_mem_bank(addr)) {
        return mem_bank->read_word(addr);
    } else {
        throw std::invalid_argument(invalid_address_msg(addr));
    }
}

void Mmu::write_byte(uint16_t addr, uint8_t byte) {
    if (IMemBank *mem_bank = get_mem_bank(addr)) {
        mem_bank->write_byte(addr, byte);
    } else {
        throw std::invalid_argument(invalid_address_msg(addr));
    }
}

void Mmu::write_word(uint16_t addr, uint16_t word) {
    if (IMemBank *mem_bank = get_mem_bank(addr)) {
        mem_bank->write_word(addr, word);
    } else {
        throw std::invalid_argument(invalid_address_msg(addr));
    }
}

} // namespace n_e_s::core
