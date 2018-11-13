// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

#include <cassert>

namespace n_e_s::core {

Cpu::Cpu(IMmu *const mmu) : registers(), mmu_(mmu), pipeline_() {
}

void Cpu::execute() {
    if (pipeline_.empty()) {
        const uint8_t opcode = mmu_->read_byte(registers.pc++);

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
            assert(false);
        }
    }

    pipeline_.front()();
    pipeline_.pop();
}

uint8_t Cpu::lsr_a() {
    set_carry(registers.a & 1);
    registers.a &= ~1;
    registers.a >>= 1;
    set_zero(registers.a);

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
    ++registers.x;
    set_zero(registers.x);
    set_negative(registers.x);
    return 2;
}

uint8_t Cpu::sed() {
    set_flag(D_FLAG);
    return 2;
}

void Cpu::clear_flag(uint8_t flag) {
    registers.p &= ~flag;
}

void Cpu::set_flag(uint8_t flag) {
    registers.p |= flag;
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
