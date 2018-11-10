// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

CPU::CPU(IMmu* const mmu) : registers(), mmu_(mmu) {
}

uint8_t CPU::clc() {
    clear_flag(C_FLAG);
    return 2;
}

uint8_t CPU::sec() {
    set_flag(C_FLAG);
    return 2;
}

uint8_t CPU::lsr_a() {
    set_carry(registers.a & 1);
    registers.a &= ~1;
    registers.a >>= 1;
    set_zero(registers.a);

    return 2;
}

uint8_t CPU::cli() {
    clear_flag(I_FLAG);
    return 2;
}

uint8_t CPU::sei() {
    set_flag(I_FLAG);
    return 2;
}

uint8_t CPU::clv() {
    clear_flag(V_FLAG);
    return 2;
}

uint8_t CPU::cld() {
    clear_flag(D_FLAG);
    return 2;
}

uint8_t CPU::nop() {
    return 2;
}

uint8_t CPU::inx() {
    ++registers.x;
    set_zero(registers.x);
    set_negative(registers.x);
    return 2;
}

uint8_t CPU::sed() {
    set_flag(D_FLAG);
    return 2;
}

void CPU::clear_flag(uint8_t flag) {
    registers.p &= ~flag;
}

void CPU::set_flag(uint8_t flag) {
    registers.p |= flag;
}

void CPU::set_carry(bool carry) {
    if (carry) {
        set_flag(C_FLAG);
    } else {
        clear_flag(C_FLAG);
    }
}

void CPU::set_zero(uint8_t byte) {
    if (byte == 0) {
        set_flag(Z_FLAG);
    } else {
        clear_flag(Z_FLAG);
    }
}

void CPU::set_negative(uint8_t byte) {
    if (byte & 1 << 7) {
        set_flag(N_FLAG);
    } else {
        clear_flag(N_FLAG);
    }
}
