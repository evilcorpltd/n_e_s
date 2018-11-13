// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace n_e_s::core {

Cpu::Cpu(Registers *const registers, IMmu *const mmu)
        : registers_(registers), mmu_(mmu), pipeline_() {
}

void Cpu::execute() {
    if (pipeline_.empty()) {
        const uint8_t opcode = mmu_->read_byte(registers_->pc++);

        switch (opcode) {
        case CLC:
            pipeline_.push([=](){ clear_flag(C_FLAG); });
            return;
        case SEC:
            pipeline_.push([=](){ set_flag(C_FLAG); });
            return;
        case NOP:
            pipeline_.push([](){ /* Do nothing. */ });
            return;
        default:
            std::stringstream err;
            err << "Bad instruction: " << std::showbase << std::hex << +opcode;
            throw std::logic_error(err.str());
        }
    }

    pipeline_.front()();
    pipeline_.pop();
}

uint8_t Cpu::lsr_a() {
    set_carry(registers_->a & 1);
    registers_->a &= ~1;
    registers_->a >>= 1;
    set_zero(registers_->a);

    return 2;
}

uint8_t Cpu::cli() {
    clear_flag(I_FLAG);
    return 2;
}

uint8_t Cpu::sei() {
    set_flag(I_FLAG);
    return 2;
}

uint8_t Cpu::clv() {
    clear_flag(V_FLAG);
    return 2;
}

uint8_t Cpu::cld() {
    clear_flag(D_FLAG);
    return 2;
}

uint8_t Cpu::inx() {
    ++registers_->x;
    set_zero(registers_->x);
    set_negative(registers_->x);
    return 2;
}

uint8_t Cpu::sed() {
    set_flag(D_FLAG);
    return 2;
}

void Cpu::clear_flag(uint8_t flag) {
    registers_->p &= ~flag;
}

void Cpu::set_flag(uint8_t flag) {
    registers_->p |= flag;
}

void Cpu::set_carry(bool carry) {
    if (carry) {
        set_flag(C_FLAG);
    } else {
        clear_flag(C_FLAG);
    }
}

void Cpu::set_zero(uint8_t byte) {
    if (byte == 0) {
        set_flag(Z_FLAG);
    } else {
        clear_flag(Z_FLAG);
    }
}

void Cpu::set_negative(uint8_t byte) {
    if (byte & 1 << 7) {
        set_flag(N_FLAG);
    } else {
        clear_flag(N_FLAG);
    }
}

}
