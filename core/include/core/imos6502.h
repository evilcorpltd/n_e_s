#pragma once

#include <cstdint>
#include <optional>

#include "core/icpu.h"
#include "core/opcode.h"

namespace n_e_s::core {

enum CpuFlag {
    C_FLAG = 1u << 0u, // carry
    Z_FLAG = 1u << 1u, // zero
    I_FLAG = 1u << 2u, // interrupt disable
    D_FLAG = 1u << 3u, // decimal mode
    B_FLAG = 1u << 4u, // break
    FLAG_5 = 1u << 5u, // unused, always 1
    V_FLAG = 1u << 6u, // overflow
    N_FLAG = 1u << 7u, // negative
};

struct CpuRegisters {
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t a; // accumulator
    uint8_t x; // x index
    uint8_t y; // y index
    uint8_t p; // status
};

struct CpuState {
    std::optional<Opcode> current_opcode;
    uint16_t start_pc{0u}; // Pc when this opcode was decoded
    uint64_t start_cycle{0u}; // Cycle count when this opcode was decoded

    uint64_t cycle{0u}; // Current cycle, increases every time the cpu executes
};

class IMos6502 : public ICpu {
public:
    [[nodiscard]] virtual const CpuState &state() const = 0;

    virtual void set_nmi(bool nmi) = 0;
};

} // namespace n_e_s::core
