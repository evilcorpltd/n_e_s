#pragma once

#include <cstdint>

namespace n_e_s::core {

enum class AddressMode {
    Invalid,
    Implied,
    Immediate,
    Zeropage,
    ZeropageX,
    ZeropageY,
    Relative,
    Absolute,
    Accumulator
};

enum class Instruction {
    Invalid,
    BRK,
    PHP,
    BPL,
    CLC,
    BMI,
    SEC,
    LSR,
    PHA,
    JMP,
    BVC,
    CLI,
    BVS,
    SEI,
    STY,
    STA,
    STX,
    TXS,
    BCC,
    LDX,
    LDY,
    LDA,
    BCS,
    CLV,
    BNE,
    CLD,
    NOP,
    INX,
    INY,
    BEQ,
    SED,
    TYA,
    TAY,
    TAX,
    TSX,
    TXA,
    DEY,
    DEX,
};

struct Opcode {
    Instruction instruction;
    AddressMode addressMode;
};

Opcode decode(const uint8_t op);

} // namespace n_e_s::core
