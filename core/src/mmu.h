// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/imembank.h"
#include "core/immu.h"

#include <cstdint>
#include <vector>

namespace n_e_s::core {

class Mmu : public IMmu {
public:
    Mmu();
    ~Mmu() = default;

    void add_mem_bank(IMemBank *mem_bank);

    uint8_t read_byte(uint16_t addr) const override;
    uint16_t read_word(uint16_t addr) const override;

    void write_byte(uint16_t addr, uint8_t byte) override;
    void write_word(uint16_t addr, uint16_t word) override;

private:
    IMemBank *get_mem_bank(uint16_t addr) const;

    std::vector<IMemBank *> mem_banks_;
};

} // namespace n_e_s::core
