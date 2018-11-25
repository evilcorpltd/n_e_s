// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mos6502.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace {

enum Opcode : uint8_t {
    BRK = 0x00,
    PHP = 0x08,
    BPL = 0x10,
    CLC = 0x18,
    BMI = 0x30,
    SEC = 0x38,
    LSR_A = 0x4A,
    PHA = 0x48,
    JMP = 0x4C,
    BVC = 0x50,
    CLI = 0x58,
    BVS = 0x70,
    SEI = 0x78,
    BCC = 0x90,
    LDY_I = 0xA0,
    BCS = 0xB0,
    CLV = 0xB8,
    BNE = 0xD0,
    CLD = 0xD8,
    NOP = 0xEA,
    INX = 0xE8,
    BEQ = 0xF0,
    SED = 0xF8,
};

const uint16_t kResetAddress = 0xFFFC; // This is where the reset routine is.
const uint16_t kBrkAddress = 0xFFFE; // This is where the break routine is.

constexpr bool is_negative(uint8_t byte) {
    return byte & (1 << 7);
}

constexpr uint8_t low_bits(uint8_t byte) {
    return byte & ~(1 << 7);
}

constexpr int8_t to_signed(uint8_t byte) {
    if (is_negative(byte)) {
        return low_bits(byte) - 128;
    }

    return low_bits(byte);
}

constexpr uint16_t high_byte(uint16_t word) {
    return word & 0xFF00;
}

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
        case PHP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                ram_.write_byte(--registers_->sp, registers_->p);
            });
            return;
        case BPL:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & N_FLAG); }));
            return;
        case CLC:
            pipeline_.push([=]() { clear_flag(C_FLAG); });
            return;
        case BMI:
            pipeline_.push(branch_on([=]() { return registers_->p & N_FLAG; }));
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
                clear_flag(N_FLAG);
            });
            return;
        case PHA:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                ram_.write_byte(--registers_->sp, registers_->a);
            });
            return;
        case JMP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                registers_->pc = mmu_->read_word(registers_->pc - 1);
            });
            return;
        case BVC:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & V_FLAG); }));
            return;
        case CLI:
            pipeline_.push([=]() { clear_flag(I_FLAG); });
            return;
        case BVS:
            pipeline_.push(branch_on([=]() { return registers_->p & V_FLAG; }));
            return;
        case SEI:
            pipeline_.push([=]() { set_flag(I_FLAG); });
            return;
        case BCC:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & C_FLAG); }));
            return;
        case LDY_I:
            pipeline_.push([=]() {
                registers_->y = mmu_->read_byte(registers_->pc++);
                set_zero(registers_->y);
                set_negative(registers_->y);
            });
            return;
        case BCS:
            pipeline_.push(branch_on([=]() { return registers_->p & C_FLAG; }));
            return;
        case CLV:
            pipeline_.push([=]() { clear_flag(V_FLAG); });
            return;
        case BNE:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & Z_FLAG); }));
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
        case BEQ:
            pipeline_.push(branch_on([=]() { return registers_->p & Z_FLAG; }));
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

void Mos6502::reset() {
    while (!pipeline_.empty()) {
        pipeline_.pop();
    }

    registers_->pc = mmu_->read_word(kResetAddress);
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
    if (is_negative(byte)) {
        set_flag(N_FLAG);
    } else {
        clear_flag(N_FLAG);
    }
}

std::function<void()> Mos6502::branch_on(std::function<bool()> condition) {
    return [=]() {
        if (!condition()) {
            ++registers_->pc;
            return;
        }

        pipeline_.push([=]() {
            const uint8_t offset = mmu_->read_byte(registers_->pc++);
            const uint16_t page = high_byte(registers_->pc);

            registers_->pc += to_signed(offset);

            if (page != high_byte(registers_->pc)) {
                // We crossed a page boundary so we spend 1 more cycle.
                pipeline_.push([=]() { /* Do nothing. */ });
            }
        });
    };
}

} // namespace n_e_s::core
