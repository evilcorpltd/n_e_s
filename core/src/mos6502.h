// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/icpu.h"
#include "core/immu.h"

#include <cstdint>
#include <functional>
#include <queue>

namespace n_e_s::core {

class Mos6502 : public ICpu {
public:
    // Assumes ownership of nothing. Really irresponsible.
    Mos6502(Registers *registers, IMmu *mmu);

    // ICpu
    void execute() override;
    void reset() override;

private:
    Registers *const registers_;
    IMmu *const mmu_;

    // Wraps the mmu to provide more convenient access to the stack.
    // All functions have side effects on the stack pointer, so it'll always
    // point to the next available address in the mmu's stack area.
    class Stack {
    public:
        Stack(Registers *registers, IMmu *mmu);

        uint8_t pop_byte();
        uint16_t pop_word();

        void push_byte(uint8_t byte);
        void push_word(uint16_t word);

    private:
        Registers *const registers_;
        IMmu *const mmu_;
        const uint16_t ram_offset_{0x0100};
    };

    Stack stack_;

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

    // Returns an atom for the cpu pipeline for branching on a condition.
    std::function<void()> branch_on(std::function<bool()> condition);
};

} // namespace n_e_s::core
