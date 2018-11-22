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

private:
    Registers *const registers_;
    IMmu *const mmu_;

    // Exactly like the mmu except that the address is offset to be inside
    // the ram bank.
    class Ram {
    public:
        Ram(IMmu *mmu);

        uint8_t read_byte(uint8_t addr) const;
        uint16_t read_word(uint8_t addr) const;

        void write_byte(uint8_t addr, uint8_t byte);
        void write_word(uint8_t addr, uint16_t word);

    private:
        IMmu *const mmu_;
        const uint16_t ram_offset_{0x0100};
    };

    Ram ram_;

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

} // namespace n_e_s::core
