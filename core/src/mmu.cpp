#include "mmu.h"

#include "nes/core/invalid_address.h"

#include <algorithm>

namespace n_e_s::core {

namespace {

auto equal(uint16_t addr) {
    return [addr](const std::unique_ptr<IMemBank> &mem_bank) {
        return mem_bank->is_address_in_range(addr);
    };
}

} // namespace

Mmu::Mmu() : mem_banks_() {}

void Mmu::clear() {
    mem_banks_.clear();
}

void Mmu::add_mem_bank(std::unique_ptr<IMemBank> mem_bank) {
    mem_banks_.push_back(std::move(mem_bank));
}

void Mmu::set_mem_banks(MemBankList mem_banks) {
    mem_banks_ = std::move(mem_banks);
}

IMemBank *Mmu::get_mem_bank(uint16_t addr) const {
    auto it = std::find_if(begin(mem_banks_), end(mem_banks_), equal(addr));
    if (it != end(mem_banks_)) {
        return (*it).get();
    }

    return nullptr;
}

uint8_t Mmu::read_byte(uint16_t addr) const {
    if (const IMemBank *mem_bank = get_mem_bank(addr)) {
        return mem_bank->read_byte(addr);
    }

    throw InvalidAddress(addr);
}

void Mmu::write_byte(uint16_t addr, uint8_t byte) {
    if (IMemBank *mem_bank = get_mem_bank(addr)) {
        mem_bank->write_byte(addr, byte);
    } else {
        throw InvalidAddress(addr);
    }
}

} // namespace n_e_s::core
