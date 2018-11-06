// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

#include <cassert>

int main() {
    CPU cpu;
    assert(cpu.registers.p == 0);

    cpu.nop();
    assert(cpu.registers.p == 0);

    cpu.sec();
    cpu.sed();
    assert(cpu.registers.p == (C_FLAG | D_FLAG));

    cpu.cld();
    assert(cpu.registers.p == C_FLAG);

    cpu.clc();
    assert(cpu.registers.p == 0);

    return 0;
}
