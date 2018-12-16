// Copyright 2018 Robin Linden <dev@robinlinden.eu>

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
    BCC,
    LDY,
    BCS,
    CLV,
    BNE,
    CLD,
    NOP,
    INX,
    BEQ,
    SED
};

struct Opcode {
    Instruction instruction;
    AddressMode addressMode;
};

Opcode decode(const uint8_t op);

} // namespace n_e_s::core
