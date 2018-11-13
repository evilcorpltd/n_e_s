// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/icpu.h"
#include "core/immu.h"

#include <cstdint>
#include <functional>
#include <queue>

namespace n_e_s::core {

class Cpu : public ICpu {
public:
    // Assumes ownership of nothing. Really irresponsible.
    Cpu(Registers *registers, IMmu *mmu);

    // ICpu
    void execute() override;

    uint8_t cli(); // 58, clear interrupt disable
    uint8_t sei(); // 78, set interrupt disable
    uint8_t clv(); // b8, clear overflow

    enum Opcode : uint8_t {
        CLC = 0x18,
        SEC = 0x38,
        LSR_A = 0x4A,
        CLD = 0xD8,
        NOP = 0xEA,
        INX = 0xE8,
        SED = 0xF8,
    };

private:
    Registers *const registers_;
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
