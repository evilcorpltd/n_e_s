#pragma once

#include <cstdint>

namespace n_e_s::core {

enum CpuFlag {
    C_FLAG = 1 << 0, // carry
    Z_FLAG = 1 << 1, // zero
    I_FLAG = 1 << 2, // interrupt disable
    D_FLAG = 1 << 3, // decimal mode
    B_FLAG = 1 << 4, // break
    FLAG_5 = 1 << 5, // unused, always 1
    V_FLAG = 1 << 6, // overflow
    N_FLAG = 1 << 7, // negative
};

class ICpu {
public:
    virtual ~ICpu() = default;

    // Steps CPU execution forward 1 cycle.
    virtual void execute() = 0;

    // Causes the CPU to jump to its reset routine.
    virtual void reset() = 0;

    struct Registers {
        uint16_t pc; // program counter
        uint8_t sp; // stack pointer
        uint8_t a; // accumulator
        uint8_t x; // x index
        uint8_t y; // y index
        uint8_t p; // status
    };
};

} // namespace n_e_s::core
