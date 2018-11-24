// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include <cstdint>

namespace n_e_s::core {

const uint8_t C_FLAG = 1 << 0; // carry
const uint8_t Z_FLAG = 1 << 1; // zero
const uint8_t I_FLAG = 1 << 2; // interrupt disable
const uint8_t D_FLAG = 1 << 3; // decimal mode
const uint8_t B_FLAG = 1 << 4; // break
const uint8_t FLAG_5 = 1 << 5; // unused, always 1
const uint8_t V_FLAG = 1 << 6; // overflow
const uint8_t N_FLAG = 1 << 7; // negative

struct Registers {
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t a; // accumulator
    uint8_t x; // x index
    uint8_t y; // y index
    uint8_t p; // status
};

class ICpu {
public:
    virtual ~ICpu() = default;

    // Steps CPU execution forward 1 cycle.
    virtual void execute() = 0;
};

} // namespace n_e_s::core
