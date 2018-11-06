// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include <cassert>
#include <cstdint>

static const uint8_t C_FLAG = 1 << 0; // carry
static const uint8_t Z_FLAG = 1 << 1; // zero
static const uint8_t I_FLAG = 1 << 2; // interrupt disable
static const uint8_t D_FLAG = 1 << 3; // decimal mode
static const uint8_t B_FLAG = 1 << 4; // break
static const uint8_t FLAG_5 = 1 << 5; // unused, always 1
static const uint8_t V_FLAG = 1 << 6; // overflow
static const uint8_t S_FLAG = 1 << 7; // negative

struct Registers {
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t a; // accumulator
    uint8_t x; // x index
    uint8_t y; // y index
    uint8_t p; // status
};

class CPU {
public:
    Registers registers;

    uint8_t clc(); // 18, clear carry
    uint8_t sec(); // 38, set carry
    uint8_t cli(); // 58, clear interrupt disable
    uint8_t sei(); // 78, set interrupt disable
    uint8_t clv(); // b8, clear overflow
    uint8_t cld(); // d8, clear decimal
    uint8_t nop(); // ea
    uint8_t sed(); // f8, set decimal

private:
    void clear_flag(uint8_t flag);
    void set_flag(uint8_t flag);
};

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

int main() {
    CPU cpu;
    assert(cpu.registers.p == 0);

    cpu.nop();
    assert(cpu.registers.p == 0);

    cpu.sec();
    cpu.sed();
    assert(cpu.registers.p == C_FLAG | D_FLAG);

    cpu.cld();
    assert(cpu.registers.p == C_FLAG);

    cpu.clc();
    assert(cpu.registers.p == 0);

    return 0;
}
