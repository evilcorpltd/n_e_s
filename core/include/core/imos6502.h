#pragma once

#include <cstdint>
#include <optional>

#include "core/icpu.h"
#include "core/opcode.h"

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

struct CpuRegisters {
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t a; // accumulator
    uint8_t x; // x index
    uint8_t y; // y index
    uint8_t p; // status
};

class IMos6502 : public ICpu {
public:
    // Returns the currently staged opcode.
    virtual std::optional<Opcode> current_opcode() const = 0;
};

} // namespace n_e_s::core
