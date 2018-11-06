// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

CPU::CPU() : registers() {
}

uint8_t CPU::clc() {
    clear_flag(C_FLAG);
    return 2;
}

uint8_t CPU::sec() {
    set_flag(C_FLAG);
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
