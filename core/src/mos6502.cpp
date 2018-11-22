// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mos6502.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace {

enum Opcode : uint8_t {
    BRK = 0x00,
    CLC = 0x18,
    SEC = 0x38,
    LSR_A = 0x4A,
    PHA = 0x48,
    JMP = 0x4C,
    CLI = 0x58,
    SEI = 0x78,
    CLV = 0xB8,
    CLD = 0xD8,
    NOP = 0xEA,
    INX = 0xE8,
    SED = 0xF8,
};

const uint16_t kBrkAddress = 0xFFFE; // This is where the break routine is.

} // namespace

namespace n_e_s::core {

Mos6502::Ram::Ram(IMmu *mmu) : mmu_(mmu) {}

uint8_t Mos6502::Ram::read_byte(uint8_t addr) const {
    return mmu_->read_byte(ram_offset_ + addr);
}

uint16_t Mos6502::Ram::read_word(uint8_t addr) const {
    return mmu_->read_word(ram_offset_ + addr);
}

void Mos6502::Ram::write_byte(uint8_t addr, uint8_t byte) {
    mmu_->write_byte(ram_offset_ + addr, byte);
}

void Mos6502::Ram::write_word(uint8_t addr, uint16_t word) {
    mmu_->write_word(ram_offset_ + addr, word);
}

Mos6502::Mos6502(Registers *const registers, IMmu *const mmu)
        : registers_(registers), mmu_(mmu), ram_(mmu_), pipeline_() {}

// Most instruction timings are from https://robinli.eu/f/6502_cpu.txt
void Mos6502::execute() {
    if (pipeline_.empty()) {
        const auto opcode =
                static_cast<Opcode>(mmu_->read_byte(registers_->pc++));

        switch (opcode) {
        case BRK:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                /* Do nothing. */
            });
            pipeline_.push([=]() {
                registers_->sp -= 2;
                ram_.write_word(registers_->sp, registers_->pc);
            });
            pipeline_.push([=]() {
                ram_.write_byte(--registers_->sp, registers_->p | B_FLAG);
            });
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push(
                    [=]() { registers_->pc = mmu_->read_word(kBrkAddress); });
            return;
        case CLC:
            pipeline_.push([=]() { clear_flag(C_FLAG); });
            return;
        case SEC:
            pipeline_.push([=]() { set_flag(C_FLAG); });
            return;
        case LSR_A:
            pipeline_.push([=]() {
                set_carry(registers_->a & 1);
                registers_->a &= ~1;
                registers_->a >>= 1;
                set_zero(registers_->a);
            });
            return;
        case PHA:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                ram_.write_byte(--registers_->sp, registers_->a);
            });
        case JMP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                registers_->pc = mmu_->read_word(registers_->pc - 1);
            });
            return;
        case CLI:
            pipeline_.push([=]() { clear_flag(I_FLAG); });
            return;
        case SEI:
            pipeline_.push([=]() { set_flag(I_FLAG); });
            return;
        case CLV:
            pipeline_.push([=]() { clear_flag(V_FLAG); });
            return;
        case CLD:
            pipeline_.push([=]() { clear_flag(D_FLAG); });
            return;
        case NOP:
            pipeline_.push([]() { /* Do nothing. */ });
            return;
        case INX:
            pipeline_.push([=]() {
                ++registers_->x;
                set_zero(registers_->x);
                set_negative(registers_->x);
            });
            return;
        case SED:
            pipeline_.push([=]() { set_flag(D_FLAG); });
            return;
        }

        std::stringstream err;
        err << "Bad instruction: " << std::showbase << std::hex << +opcode;
        throw std::logic_error(err.str());
    }

    pipeline_.front()();
    pipeline_.pop();
}

void Mos6502::clear_flag(uint8_t flag) {
    registers_->p &= ~flag;
}

void Mos6502::set_flag(uint8_t flag) {
    registers_->p |= flag;
}

void Mos6502::set_carry(bool carry) {
    if (carry) {
        set_flag(C_FLAG);
    } else {
        clear_flag(C_FLAG);
    }
}

void Mos6502::set_zero(uint8_t byte) {
    if (byte == 0) {
        set_flag(Z_FLAG);
    } else {
        clear_flag(Z_FLAG);
    }
}

void Mos6502::set_negative(uint8_t byte) {
    if (byte & 1 << 7) {
        set_flag(N_FLAG);
    } else {
        clear_flag(N_FLAG);
    }
}

} // namespace n_e_s::core
