// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"

#include <cstdint>

namespace n_e_s::core {

class Mmu : public IMmu {
public:
    Mmu() = default;

    uint8_t read_byte(uint16_t addr) const override;
    uint16_t read_word(uint16_t addr) const override;

    void write_byte(uint16_t addr, uint8_t byte) override;
    void write_word(uint16_t addr, uint16_t word) override;

private:
    // Translates a raw address into a pointer into one of the different memory
    // banks.
    uint8_t *translate_address(uint16_t addr);
    const uint8_t *translate_address(uint16_t addr) const;

    constexpr static uint16_t ram_size_ = 0x800;
    constexpr static uint16_t ram_end_ = 0x1FFF;
    uint8_t ram_[ram_size_] {}; // 0x0000 - 0x1FFF, repeats every 0x800 byte.

    constexpr static uint16_t ppu_size_ = 0x8;
    constexpr static uint16_t ppu_end_ = 0x3FFF;
    uint8_t ppu_[ppu_size_] {}; // 0x2000 - 0x3FFF, repeats every 0x8 byte.

    constexpr static uint16_t io_size_ = 0x18;
    constexpr static uint16_t io_end_ = 0x4017;
    uint8_t io_[io_size_] {}; // 0x4000 - 0x4017

    constexpr static uint16_t io_dev_size_ = 0x8;
    constexpr static uint16_t io_dev_end_ = 0x401F;
    uint8_t io_dev_[io_dev_size_] {}; // 0x4018 - 401F

    constexpr static uint16_t cartridge_size_ = 0xBFE0;
    uint8_t cartridge_[cartridge_size_] {}; // 0x4020 - 0xFFFF
};

}
