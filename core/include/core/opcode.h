#pragma once

#include <cstdint>
#include <string_view>

namespace n_e_s::core {

enum Instruction : uint8_t {
    BrkImplied = 0x00,
    OraIndexedIndirect = 0x01,
    NopZeropage04 = 0x04, // Undocumented
    OraZeropage = 0x05,
    AslZeropage = 0x06,
    PhpImplied = 0x08,
    OraImmediate = 0x09,
    AslAccumulator = 0x0A,
    NopAbsolute0C = 0x0C, // Undocumented
    OraAbsolute = 0x0D,
    AslAbsolute = 0x0E,
    BplRelative = 0x10,
    OraIndirectIndexed = 0x11,
    NopZeropageX14 = 0x14, // Undocumented
    OraZeropageX = 0x15,
    AslZeropageX = 0x16,
    ClcImplied = 0x18,
    OraAbsoluteY = 0x19,
    NopImplied1A = 0x1A, // Undocumented
    NopAbsoluteX1C = 0x1C, // Undocumented
    OraAbsoluteX = 0x1D,
    AslAbsoluteX = 0x1E,
    JsrAbsolute = 0x20,
    AndIndirectX = 0x21,
    BitZeropage = 0x24,
    AndZeropage = 0x25,
    RolZeropage = 0x26,
    PlpImplied = 0x28,
    AndImmediate = 0x29,
    RolAccumulator = 0x2A,
    BitAbsolute = 0x2C,
    AndAbsolute = 0x2D,
    RolAbsolute = 0x2E,
    BmiRelative = 0x30,
    AndIndirectY = 0x31,
    NopZeropageX34 = 0x34, // Undocumented
    AndZeropageX = 0x35,
    RolZeropageX = 0x36,
    SecImplied = 0x38,
    AndAbsoluteY = 0x39,
    NopImplied3A = 0x3A, // Undocumented
    NopAbsoluteX3C = 0x3C, // Undocumented
    AndAbsoluteX = 0x3D,
    RolAbsoluteX = 0x3E,
    RtiImplied = 0x40,
    EorIndirectX = 0x41,
    NopZeropage44 = 0x44, // Undocumented
    EorZeropage = 0x45,
    LsrZeropage = 0x46,
    PhaImplied = 0x48,
    EorImmediate = 0x49,
    LsrAccumulator = 0x4A,
    JmpAbsolute = 0x4C,
    EorAbsolute = 0x4D,
    LsrAbsolute = 0x4E,
    BvcRelative = 0x50,
    EorIndirectY = 0x51,
    NopZeropageX54 = 0x54, // Undocumented
    EorZeropageX = 0x55,
    LsrZeropageX = 0x56,
    CliImplied = 0x58,
    EorAbsoluteY = 0x59,
    NopImplied5A = 0x5A, // Undocumented
    NopAbsoluteX5C = 0x5C, // Undocumented
    EorAbsoluteX = 0x5D,
    LsrAbsoluteX = 0x5E,
    RtsImplied = 0x60,
    AdcIndirectX = 0x61,
    NopZeropage64 = 0x64, // Undocumented
    AdcZeropage = 0x65,
    RorZeropage = 0x66,
    PlaImplied = 0x68,
    AdcImmediate = 0x69,
    RorAccumulator = 0x6A,
    JmpIndirect = 0x6C,
    AdcAbsolute = 0x6D,
    RorAbsolute = 0x6E,
    BvsRelative = 0x70,
    AdcIndirectY = 0x71,
    NopZeropageX74 = 0x74, // Undocumented
    AdcZeropageX = 0x75,
    RorZeropageX = 0x76,
    SeiImplied = 0x78,
    AdcAbsoluteY = 0x79,
    NopImplied7A = 0x7A, // Undocumented
    NopAbsoluteX7C = 0x7C, // Undocumented
    AdcAbsoluteX = 0x7D,
    RorAbsoluteX = 0x7E,
    NopImmediate80 = 0x80, // Undocumented
    StaIndexedIndirect = 0x81,
    SaxIndirectX = 0x83, // Undocumented
    StyZeropage = 0x84,
    StaZeropage = 0x85,
    StxZeropage = 0x86,
    SaxZeropage = 0x87, // Undocumented
    DeyImplied = 0x88,
    TxaImplied = 0x8A,
    StyAbsolute = 0x8C,
    StaAbsolute = 0x8D,
    StxAbsolute = 0x8E,
    SaxAbsolute = 0x8F, // Undocumented
    BccRelative = 0x90,
    StaIndirectIndexed = 0x91,
    StyZeropageX = 0x94,
    StaZeropageX = 0x95,
    StxZeropageY = 0x96,
    SaxZeropageY = 0x97, // Undocumented
    TyaImplied = 0x98,
    StaAbsoluteY = 0x99,
    TxsImplied = 0x9A,
    StaAbsoluteX = 0x9D,
    LdyImmediate = 0xA0,
    LdaIndirectX = 0xA1,
    LdxImmediate = 0xA2,
    LaxIndirectX = 0xA3, // Undocumented
    LdyZeropage = 0xA4,
    LdaZeropage = 0xA5,
    LdxZeropage = 0xA6,
    LaxZeropage = 0xA7, // Undocumented
    TayImplied = 0xA8,
    LdaImmediate = 0xA9,
    TaxImplied = 0xAA,
    LdyAbsolute = 0xAC,
    LdaAbsolute = 0xAD,
    LdxAbsolute = 0xAE,
    LaxAbsolute = 0xAF, // Undocumented
    BcsRelative = 0xB0,
    LdaIndirectY = 0xB1,
    LaxIndirectY = 0xB3, // Undocumented
    LdyZeropageX = 0xB4,
    LdaZeropageX = 0xB5,
    LdxZeropageY = 0xB6,
    LaxZeropageY = 0xB7, // Undocumented
    ClvImplied = 0xB8,
    LdaAbsoluteY = 0xB9,
    TsxImplied = 0xBA,
    LdyAbsoluteX = 0xBC,
    LdaAbsoluteX = 0xBD,
    LdxAbsoluteY = 0xBE,
    LaxAbsoluteY = 0xBF, // Undocumented
    CpyImmediate = 0xC0,
    CmpIndirectX = 0xC1,
    CpyZeropage = 0xC4,
    CmpZeropage = 0xC5,
    DecZeropage = 0xC6,
    InyImplied = 0xC8,
    CmpImmediate = 0xC9,
    DexImplied = 0xCA,
    CpyAbsolute = 0xCC,
    CmpAbsolute = 0xCD,
    DecAbsolute = 0xCE,
    BneRelative = 0xD0,
    CmpIndirectY = 0xD1,
    NopZeropageXD4 = 0xD4, // Undocumented
    CmpZeropageX = 0xD5,
    DecZeropageX = 0xD6,
    CldImplied = 0xD8,
    CmpAbsoluteY = 0xD9,
    NopImpliedDA = 0xDA, // Undocumented
    NopAbsoluteXDC = 0xDC, // Undocumented
    CmpAbsoluteX = 0xDD,
    DecAbsoluteX = 0xDE,
    CpxImmediate = 0xE0,
    SbcIndirectX = 0xE1,
    CpxZeropage = 0xE4,
    SbcZeropage = 0xE5,
    IncZeropage = 0xE6,
    InxImplied = 0xE8,
    SbcImmediate = 0xE9,
    NopImplied = 0xEA,
    SbcImmediateEB = 0xEB, // Undocumented
    CpxAbsolute = 0xEC,
    SbcAbsolute = 0xED,
    IncAbsolute = 0xEE,
    BeqRelative = 0xF0,
    SbcIndirectY = 0xF1,
    NopZeropageXF4 = 0xF4, // Undocumented
    SbcZeropageX = 0xF5,
    IncZeropageX = 0xF6,
    SedImplied = 0xF8,
    SbcAbsoluteY = 0xF9,
    NopImpliedFA = 0xFA, // Undocumented
    NopAbsoluteXFC = 0xFC, // Undocumented
    SbcAbsoluteX = 0xFD,
    IncAbsoluteX = 0xFE,
};

enum class Family {
    Invalid,
    BRK,
    ASL,
    INC,
    PHP,
    BPL,
    CLC,
    BIT,
    PLP,
    AND,
    RTI,
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
    DEC,
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
    EOR,
    ROL,
    ROR,
    ORA,
    SBC,
    LAX,
    SAX,
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
    IndirectIndexed,
    Indirect
};

enum class MemoryAccess { Read, Write, ReadWrite };

struct Opcode {
    Family family;
    Instruction instruction;
    AddressMode address_mode;
};

[[nodiscard]] Opcode decode(const uint8_t op);

[[nodiscard]] MemoryAccess get_memory_access(const Family family);

[[nodiscard]] std::string_view to_string(const Family family);

} // namespace n_e_s::core
