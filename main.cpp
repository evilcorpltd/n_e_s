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

    cpu.lsr_a();
    assert(cpu.registers.p == Z_FLAG);

    assert(cpu.registers.x == 0);
    cpu.inx();
    assert(cpu.registers.x == 1);
    assert(cpu.registers.p == 0);
    cpu.inx();
    cpu.inx();
    assert(cpu.registers.x == 3);
    assert(cpu.registers.p == 0);
    cpu.registers.x = 126;
    cpu.inx();
    assert(cpu.registers.x == 127);
    assert(cpu.registers.p == 0);
    cpu.inx();
    assert(cpu.registers.p == N_FLAG);
    cpu.registers.x = 255;
    cpu.inx();
    assert(cpu.registers.x == 0);
    assert(cpu.registers.p == Z_FLAG);

    return 0;
}
