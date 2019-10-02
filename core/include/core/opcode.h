#pragma once

#include <cstdint>
#include <string>

namespace n_e_s::core {

enum Instruction : uint8_t {
    BrkImplied = 0x00,
    // OraZeropage = 0x05,
    // AslZeropage = 0x06,
    PhpImplied = 0x08,
    // AslAccumulator = 0x0A,
    // OraAbsolute = 0x0D,
    // AslAbsolute = 0x0E,
    BplRelative = 0x10,
    // OraZeropageX = 0x15,
    // AslZeropageX = 0x16,
    ClcImplied = 0x18,
    // OraAbsoluteY = 0x19,
    // OraAbsoluteX = 0x1D,
    // AslAbsoluteX = 0x1E,
    JsrAbsolute = 0x20,
    // AndIndirectX = 0x21,
    BitZeropage = 0x24,
    // AndZeropage = 0x25,
    // RolZeropage = 0x26,
    PlpImplied = 0x28,
    AndImmediate = 0x29,
    // RolAccumulator = 0x2A,
    BitAbsolute = 0x2C,
    AndAbsolute = 0x2D,
    // RolAbsolute = 0x2E,
    BmiRelative = 0x30,
    // AndIndirectY = 0x31,
    // AndZeropageX = 0x35,
    // RolZeropageX = 0x36,
    SecImplied = 0x38,
    AndAbsoluteY = 0x39,
    AndAbsoluteX = 0x3D,
    // RolAbsoluteX = 0x3E,
    // RtiImplied = 0x40,
    // EorIndirectX = 0x41,
    // EorZeropage = 0x45,
    // LsrAccumulator = 0x46,
    PhaImplied = 0x48,
    // EorImmediate = 0x49,
    LsrAccumulator = 0x4A,
    JmpAbsolute = 0x4C,
    // EorAbsolute = 0x4D,
    // LsrAbsolute = 0x4E,
    BvcRelative = 0x50,
    // EorIndirectY = 0x51,
    // EorZeropageX = 0x55,
    // LsrZeropageX = 0x56,
    CliImplied = 0x58,
    // EorAbsoluteY = 0x59,
    // EorAbsoluteX = 0x5D,
    // LsrAbsoluteX = 0x5E,
    RtsImplied = 0x60,
    // AdcIndirectX = 0x61,
    AdcZeropage = 0x65,
    // RorZeropage = 0x66,
    PlaImplied = 0x68,
    AdcImmediate = 0x69,
    // RorAccumulator = 0x6A,
    // JmpIndirect = 0x6C,
    AdcAbsolute = 0x6D,
    // RorAbsolute = 0x6E,
    BvsRelative = 0x70,
    // AdcIndirectY = 0x71,
    // AdcZeropageX = 0x75,
    // RorZeropageX = 0x76,
    SeiImplied = 0x78,
    AdcAbsoluteY = 0x79,
    AdcAbsoluteX = 0x7D,
    // RorAbsoluteX = 0x7E,
    StaIndexedIndirect = 0x81,
    StyZeropage = 0x84,
    StaZeropage = 0x85,
    StxZeropage = 0x86,
    DeyImplied = 0x88,
    TxaImplied = 0x8A,
    StyAbsolute = 0x8C,
    StaAbsolute = 0x8D,
    StxAbsolute = 0x8E,
    BccRelative = 0x90,
    StaIndirectIndexed = 0x91,
    StyZeropageX = 0x94,
    StaZeropageX = 0x95,
    StxZeropageY = 0x96,
    TyaImplied = 0x98,
    StaAbsoluteY = 0x99,
    TxsImplied = 0x9A,
    StaAbsoluteX = 0x9D,
    LdyImmediate = 0xA0,
    // LdaIndirectX = 0xA1,
    LdxImmediate = 0xA2,
    LdyZeropage = 0xA4,
    LdaZeropage = 0xA5,
    LdxZeropage = 0xA6,
    TayImplied = 0xA8,
    LdaImmediate = 0xA9,
    TaxImplied = 0xAA,
    LdyAbsolute = 0xAC,
    LdaAbsolute = 0xAD,
    LdxAbsolute = 0xAE,
    BcsRelative = 0xB0,
    // LdaIndirectY = 0xB1,
    LdyZeropageX = 0xB4,
    LdaZeropageX = 0xB5,
    LdxZeropageY = 0xB6,
    ClvImplied = 0xB8,
    LdaAbsoluteY = 0xB9,
    TsxImplied = 0xBA,
    LdyAbsoluteX = 0xBC,
    LdaAbsoluteX = 0xBD,
    LdxAbsoluteY = 0xBE,
    CpyImmediate = 0xC0,
    // CmpIndirectX = 0xC1,
    CpyZeropage = 0xC4,
    CmpZeropage = 0xC5,
    // DecZeropage = 0xC6,
    InyImplied = 0xC8,
    CmpImmediate = 0xC9,
    DexImplied = 0xCA,
    CpyAbsolute = 0xCC,
    CmpAbsolute = 0xCD,
    // DecAbsolute = 0xCE,
    BneRelative = 0xD0,
    // CmpIndirectY = 0xD1,
    CmpZeropageX = 0xD5,
    // DecZeropageX = 0xD6,
    CldImplied = 0xD8,
    CmpAbsoluteY = 0xD9,
    CmpAbsoluteX = 0xDD,
    // DecAbsoluteX = 0xDE,
    CpxImmediate = 0xE0,
    // SbcIndirectX = 0xE1,
    CpxZeropage = 0xE4,
    // SbcZeropage = 0xE5,
    IncZeropage = 0xE6,
    InxImplied = 0xE8,
    // SbcImmediate = 0xE9,
    NopImplied = 0xEA,
    CpxAbsolute = 0xEC,
    // SbcAbsolute = 0xED,
    // IncAbsolute = 0xEE,
    BeqRelative = 0xF0,
    // SbcIndirectY = 0xF1,
    // SbcZeropageX = 0xF5,
    // IncZeropageX = 0xF6,
    SedImplied = 0xF8,
    // SbcAbsoluteY = 0xF9,
    // SbcAbsoluteX = 0xFD,
    // IncAbsoluteX = 0xFE,
};

enum class Family {
    Invalid,
    BRK,
    INC,
    PHP,
    BPL,
    CLC,
    BIT,
    PLP,
    AND,
    JSR,
    BMI,
    SEC,
    LSR,
    PHA,
    JMP,
    BVC,
    CLI,
    ADC,
    PLA,
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
    CMP,
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

enum class AddressMode {
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

struct Opcode {
    Family family;
    Instruction instruction;
    AddressMode address_mode;
};

Opcode decode(const uint8_t op);

std::string to_string(const Family family);

} // namespace n_e_s::core