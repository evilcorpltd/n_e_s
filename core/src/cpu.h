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

    uint8_t lsr_a(); // 4a, logical shift right (accumulator)
    uint8_t cli(); // 58, clear interrupt disable
    uint8_t sei(); // 78, set interrupt disable
    uint8_t clv(); // b8, clear overflow
    uint8_t cld(); // d8, clear decimal
    uint8_t inx(); // e8, increment x index
    uint8_t sed(); // f8, set decimal

    enum Opcode : uint8_t {
        CLC = 0x18,
        SEC = 0x38,
        NOP = 0xEA,
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
