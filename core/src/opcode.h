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
    AbsoluteX,
    AbsoluteY,
    Accumulator,
    IndexedIndirect,
    IndirectIndexed
};

enum class Instruction {
    Invalid,
    BRK,
    PHP,
    BPL,
    CLC,
    BIT,
    JSR,
    BMI,
    SEC,
    LSR,
    PHA,
    JMP,
    BVC,
    CLI,
    ADC,
    RTS,
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
    CPX,
    NOP,
    INX,
    INY,
    CPY,
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
