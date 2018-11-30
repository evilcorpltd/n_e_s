// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mos6502.h"

#include "opcode.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace {

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

Mos6502::Stack::Stack(Registers *registers, IMmu *mmu)
        : registers_(registers), mmu_(mmu) {}

uint8_t Mos6502::Stack::pop_byte() {
    return mmu_->read_byte(ram_offset_ + ++registers_->sp);
}

uint16_t Mos6502::Stack::pop_word() {
    const uint16_t ret = mmu_->read_word(ram_offset_ + ++registers_->sp);
    ++registers_->sp;
    return ret;
}

void Mos6502::Stack::push_byte(uint8_t byte) {
    mmu_->write_byte(ram_offset_ + registers_->sp--, byte);
}

void Mos6502::Stack::push_word(uint16_t word) {
    mmu_->write_word(ram_offset_ + --registers_->sp, word);
    --registers_->sp;
}

Mos6502::Mos6502(Registers *const registers, IMmu *const mmu)
        : registers_(registers),
          mmu_(mmu),
          stack_(registers_, mmu_),
          pipeline_() {}

// Most instruction timings are from https://robinli.eu/f/6502_cpu.txt
void Mos6502::execute() {
    if (pipeline_.empty()) {
        const auto raw_opcode{mmu_->read_byte(registers_->pc++)};
        const auto opcode = decode(raw_opcode);

        switch (opcode.instruction) {
        case Instruction::BRK:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                /* Do nothing. */
            });
            pipeline_.push([=]() { stack_.push_word(registers_->pc); });
            pipeline_.push([=]() { stack_.push_byte(registers_->p | B_FLAG); });
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push(
                    [=]() { registers_->pc = mmu_->read_word(kBrkAddress); });
            return;
        case Instruction::PHP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() { stack_.push_byte(registers_->p); });
            return;
        case Instruction::BPL:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & N_FLAG); }));
            return;
        case Instruction::CLC:
            pipeline_.push([=]() { clear_flag(C_FLAG); });
            return;
        case Instruction::BMI:
            pipeline_.push(branch_on([=]() { return registers_->p & N_FLAG; }));
            return;
        case Instruction::SEC:
            pipeline_.push([=]() { set_flag(C_FLAG); });
            return;
        case Instruction::LSR:
            if (opcode.addressMode == AddressMode::Accumulator) {
                pipeline_.push([=]() {
                    set_carry(registers_->a & 1);
                    registers_->a &= ~1;
                    registers_->a >>= 1;
                    set_zero(registers_->a);
                    clear_flag(N_FLAG);
                });
                return;
            }
            break;
        case Instruction::PHA:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() { stack_.push_byte(registers_->a); });
            return;
        case Instruction::JMP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                registers_->pc = mmu_->read_word(registers_->pc - 1);
            });
            return;
        case Instruction::BVC:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & V_FLAG); }));
            return;
        case Instruction::CLI:
            pipeline_.push([=]() { clear_flag(I_FLAG); });
            return;
        case Instruction::BVS:
            pipeline_.push(branch_on([=]() { return registers_->p & V_FLAG; }));
            return;
        case Instruction::SEI:
            pipeline_.push([=]() { set_flag(I_FLAG); });
            return;
        case Instruction::STY:
            if (opcode.addressMode == AddressMode::Absolute) {
                pipeline_.push([=]() { ++registers_->pc; });
                pipeline_.push([=]() { ++registers_->pc; });
                pipeline_.push(store_byte_abs_addr(registers_->y));
                return;
            }
            break;
        case Instruction::STA:
            if (opcode.addressMode == AddressMode::Absolute) {
                pipeline_.push([=]() { ++registers_->pc; });
                pipeline_.push([=]() { ++registers_->pc; });
                pipeline_.push(store_byte_abs_addr(registers_->a));
                return;
            }
            break;
        case Instruction::STX:
            if (opcode.addressMode == AddressMode::Absolute) {
                pipeline_.push([=]() { ++registers_->pc; });
                pipeline_.push([=]() { ++registers_->pc; });
                pipeline_.push(store_byte_abs_addr(registers_->x));
                return;
            }
            break;
        case Instruction::BCC:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & C_FLAG); }));
            return;
        case Instruction::LDY:
            if (opcode.addressMode == AddressMode::Immediate) {
                pipeline_.push([=]() {
                    registers_->y = mmu_->read_byte(registers_->pc++);
                    set_zero(registers_->y);
                    set_negative(registers_->y);
                });
                return;
            }
            break;
        case Instruction::BCS:
            pipeline_.push(branch_on([=]() { return registers_->p & C_FLAG; }));
            return;
        case Instruction::CLV:
            pipeline_.push([=]() { clear_flag(V_FLAG); });
            return;
        case Instruction::BNE:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & Z_FLAG); }));
            return;
        case Instruction::CLD:
            pipeline_.push([=]() { clear_flag(D_FLAG); });
            return;
        case Instruction::NOP:
            pipeline_.push([]() { /* Do nothing. */ });
            return;
        case Instruction::INX:
            pipeline_.push([=]() {
                ++registers_->x;
                set_zero(registers_->x);
                set_negative(registers_->x);
            });
            return;
        case Instruction::BEQ:
            pipeline_.push(branch_on([=]() { return registers_->p & Z_FLAG; }));
            return;
        case Instruction::SED:
            pipeline_.push([=]() { set_flag(D_FLAG); });
            return;
        case Instruction::Invalid:
            break;
        }
        std::stringstream err;
        err << "Bad instruction: " << std::showbase << std::hex << +raw_opcode;
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

std::function<void()> Mos6502::store_byte_abs_addr(uint8_t byte) {
    return [=]() {
        uint16_t addr = mmu_->read_word(registers_->pc - 2);
        mmu_->write_byte(addr, byte);
    };
}

} // namespace n_e_s::core
