// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/icpu.h"
#include "core/immu.h"

#include <cstdint>
#include <functional>
#include <queue>

namespace n_e_s::core {

static const uint8_t C_FLAG = 1 << 0; // carry
static const uint8_t Z_FLAG = 1 << 1; // zero
static const uint8_t I_FLAG = 1 << 2; // interrupt disable
static const uint8_t D_FLAG = 1 << 3; // decimal mode
static const uint8_t B_FLAG = 1 << 4; // break
static const uint8_t FLAG_5 = 1 << 5; // unused, always 1
static const uint8_t V_FLAG = 1 << 6; // overflow
static const uint8_t N_FLAG = 1 << 7; // negative

struct Registers {
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t a; // accumulator
    uint8_t x; // x index
    uint8_t y; // y index
    uint8_t p; // status
};

class Cpu : public ICpu {
public:
    // Assumes ownership of nothing. Really irresponsible.
    Cpu(IMmu *mmu);

    Registers registers;

    // ICpu
    void execute() override;

    uint8_t clc(); // 18, clear carry
    uint8_t sec(); // 38, set carry
    uint8_t lsr_a(); // 4a, logical shift right (accumulator)
    uint8_t cli(); // 58, clear interrupt disable
    uint8_t sei(); // 78, set interrupt disable
    uint8_t clv(); // b8, clear overflow
    uint8_t cld(); // d8, clear decimal
    uint8_t inx(); // e8, increment x index
    uint8_t nop(); // ea
    uint8_t sed(); // f8, set decimal

private:
    IMmu *const mmu_;

    // Holds the atoms staged to be executed.
    std::queue<std::function<void()>> pipeline_;

    void clear_flag(uint8_t flag);
    void set_flag(uint8_t flag);

    // sets the C flag
    void set_carry(bool carry);

    // sets the Z flag if the byte is 0
    void set_zero(uint8_t byte);

    // sets the N flag if bit 7 of the byte is set
    void set_negative(uint8_t byte);
};

}
