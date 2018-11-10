// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"

#include <cstdint>

namespace n_e_s::core {

class Mmu : public IMmu {
public:
    Mmu();

    uint8_t read_byte(uint16_t addr) const override;
    uint16_t read_word(uint16_t addr) const override;

    void write_byte(uint16_t addr, uint8_t byte) override;
    void write_word(uint16_t addr, uint16_t word) override;

private:
    uint8_t memory_[0xFFFF];
};

}
