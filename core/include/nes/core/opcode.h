#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace n_e_s::core {

enum Instruction : uint8_t {
    BrkImplied = 0x00,
    OraIndexedIndirect = 0x01,
    SloIndexedIndirect = 0x03, // Undocumented
    NopZeropage04 = 0x04, // Undocumented
    OraZeropage = 0x05,
    AslZeropage = 0x06,
    SloZeropage = 0x07, // Undocumented
    PhpImplied = 0x08,
    OraImmediate = 0x09,
    AslAccumulator = 0x0A,
    NopAbsolute0C = 0x0C, // Undocumented
    OraAbsolute = 0x0D,
    AslAbsolute = 0x0E,
    SloAbsolute = 0x0F, // Undocumented
    BplRelative = 0x10,
    OraIndirectIndexed = 0x11,
    SloIndirectIndexed = 0x13, // Undocumented
    NopZeropageX14 = 0x14, // Undocumented
    OraZeropageX = 0x15,
    AslZeropageX = 0x16,
    SloZeropageX = 0x17, // Undocumented
    ClcImplied = 0x18,
    OraAbsoluteY = 0x19,
    NopImplied1A = 0x1A, // Undocumented
    SloAbsoluteY = 0x1B, // Undocumented
    NopAbsoluteX1C = 0x1C, // Undocumented
    OraAbsoluteX = 0x1D,
    AslAbsoluteX = 0x1E,
    SloAbsoluteX = 0x1F, // Undocumented
    JsrAbsolute = 0x20,
    AndIndirectX = 0x21,
    RlaIndexedIndirect = 0x23, // Undocumented
    BitZeropage = 0x24,
    AndZeropage = 0x25,
    RolZeropage = 0x26,
    RlaZeropage = 0x27, // Undocumented
    PlpImplied = 0x28,
    AndImmediate = 0x29,
    RolAccumulator = 0x2A,
    BitAbsolute = 0x2C,
    AndAbsolute = 0x2D,
    RolAbsolute = 0x2E,
    RlaAbsolute = 0x2F, // Undocumented
    BmiRelative = 0x30,
    AndIndirectY = 0x31,
    RlaIndirectIndexed = 0x33, // Undocumented
    NopZeropageX34 = 0x34, // Undocumented
    AndZeropageX = 0x35,
    RolZeropageX = 0x36,
    RlaZeropageX = 0x37, // Undocumented
    SecImplied = 0x38,
    AndAbsoluteY = 0x39,
    NopImplied3A = 0x3A, // Undocumented
    RlaAbsoluteY = 0x3B, // Undocumented
    NopAbsoluteX3C = 0x3C, // Undocumented
    AndAbsoluteX = 0x3D,
    RolAbsoluteX = 0x3E,
    RlaAbsoluteX = 0x3F, // Undocumented
    RtiImplied = 0x40,
    EorIndirectX = 0x41,
    SreIndexedIndirect = 0x43, // Undocumented
    NopZeropage44 = 0x44, // Undocumented
    EorZeropage = 0x45,
    LsrZeropage = 0x46,
    SreZeropage = 0x47, // Undocumented
    PhaImplied = 0x48,
    EorImmediate = 0x49,
    LsrAccumulator = 0x4A,
    JmpAbsolute = 0x4C,
    EorAbsolute = 0x4D,
    LsrAbsolute = 0x4E,
    SreAbsolute = 0x4F, // Undocumented
    BvcRelative = 0x50,
    EorIndirectY = 0x51,
    SreIndirectIndexed = 0x53, // Undocumented
    NopZeropageX54 = 0x54, // Undocumented
    EorZeropageX = 0x55,
    LsrZeropageX = 0x56,
    SreZeropageX = 0x57, // Undocumented
    CliImplied = 0x58,
    EorAbsoluteY = 0x59,
    NopImplied5A = 0x5A, // Undocumented
    SreAbsoluteY = 0x5B, // Undocumented
    NopAbsoluteX5C = 0x5C, // Undocumented
    EorAbsoluteX = 0x5D,
    LsrAbsoluteX = 0x5E,
    SreAbsoluteX = 0x5F, // Undocumented
    RtsImplied = 0x60,
    AdcIndirectX = 0x61,
    RraIndexedIndirect = 0x63, // Undocumented
    NopZeropage64 = 0x64, // Undocumented
    AdcZeropage = 0x65,
    RorZeropage = 0x66,
    RraZeropage = 0x67, // Undocumented
    PlaImplied = 0x68,
    AdcImmediate = 0x69,
    RorAccumulator = 0x6A,
    JmpIndirect = 0x6C,
    AdcAbsolute = 0x6D,
    RorAbsolute = 0x6E,
    RraAbsolute = 0x6F, // Undocumented
    BvsRelative = 0x70,
    AdcIndirectY = 0x71,
    RraIndirectIndexed = 0x73, // Undocumented
    NopZeropageX74 = 0x74, // Undocumented
    AdcZeropageX = 0x75,
    RorZeropageX = 0x76,
    RraZeropageX = 0x77, // Undocumented
    SeiImplied = 0x78,
    AdcAbsoluteY = 0x79,
    NopImplied7A = 0x7A, // Undocumented
    RraAbsoluteY = 0x7B, // Undocumented
    NopAbsoluteX7C = 0x7C, // Undocumented
    AdcAbsoluteX = 0x7D,
    RorAbsoluteX = 0x7E,
    RraAbsoluteX = 0x7F, // Undocumented
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
    DcpIndexedIndirect = 0xC3, // Undocumented
    CpyZeropage = 0xC4,
    CmpZeropage = 0xC5,
    DecZeropage = 0xC6,
    DcpZeropage = 0xC7, // Undocumented
    InyImplied = 0xC8,
    CmpImmediate = 0xC9,
    DexImplied = 0xCA,
    CpyAbsolute = 0xCC,
    CmpAbsolute = 0xCD,
    DecAbsolute = 0xCE,
    DcpAbsolute = 0xCF, // Undocumented
    BneRelative = 0xD0,
    CmpIndirectY = 0xD1,
    DcpIndirectIndexed = 0xD3, // Undocumented
    NopZeropageXD4 = 0xD4, // Undocumented
    CmpZeropageX = 0xD5,
    DecZeropageX = 0xD6,
    DcpZeropageX = 0xD7, // Undocumented
    CldImplied = 0xD8,
    CmpAbsoluteY = 0xD9,
    NopImpliedDA = 0xDA, // Undocumented
    DcpAbsoluteY = 0xDB, // Undocumented
    NopAbsoluteXDC = 0xDC, // Undocumented
    CmpAbsoluteX = 0xDD,
    DecAbsoluteX = 0xDE,
    DcpAbsoluteX = 0xDF, // Undocumented
    CpxImmediate = 0xE0,
    SbcIndirectX = 0xE1,
    IsbIndexedIndirect = 0xE3, // Undocumented
    CpxZeropage = 0xE4,
    SbcZeropage = 0xE5,
    IncZeropage = 0xE6,
    IsbZeropage = 0xE7, // Undocumented
    InxImplied = 0xE8,
    SbcImmediate = 0xE9,
    NopImplied = 0xEA,
    SbcImmediateEB = 0xEB, // Undocumented
    CpxAbsolute = 0xEC,
    SbcAbsolute = 0xED,
    IncAbsolute = 0xEE,
    IsbAbsolute = 0xEF, // Undocumented
    BeqRelative = 0xF0,
    SbcIndirectY = 0xF1,
    IsbIndirectIndexed = 0xF3, // Undocumented
    NopZeropageXF4 = 0xF4, // Undocumented
    SbcZeropageX = 0xF5,
    IncZeropageX = 0xF6,
    IsbZeropageX = 0xF7, // Undocumented
    SedImplied = 0xF8,
    SbcAbsoluteY = 0xF9,
    NopImpliedFA = 0xFA, // Undocumented
    IsbAbsoluteY = 0xFB, // Undocumented
    NopAbsoluteXFC = 0xFC, // Undocumented
    SbcAbsoluteX = 0xFD,
    IncAbsoluteX = 0xFE,
    IsbAbsoluteX = 0xFF, // Undocumented
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
    DCP,
    ISB,
    SLO,
    RLA,
    SRE,
    RRA,
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

[[nodiscard]] constexpr Opcode decode(const uint8_t op) {
    switch (op) {
    case BrkImplied:
        return {Family::BRK, BrkImplied, AddressMode::Implied};
    case OraIndexedIndirect:
        return {Family::ORA, OraIndexedIndirect, AddressMode::IndexedIndirect};
    case NopZeropage04:
        return {Family::NOP, NopZeropage04, AddressMode::Zeropage};
    case AslZeropage:
        return {Family::ASL, AslZeropage, AddressMode::Zeropage};
    case PhpImplied:
        return {Family::PHP, PhpImplied, AddressMode::Implied};
    case AslAccumulator:
        return {Family::ASL, AslAccumulator, AddressMode::Accumulator};
    case NopAbsolute0C:
        return {Family::NOP, NopAbsolute0C, AddressMode::Absolute};
    case AslAbsolute:
        return {Family::ASL, AslAbsolute, AddressMode::Absolute};
    case BplRelative:
        return {Family::BPL, BplRelative, AddressMode::Relative};
    case NopZeropageX14:
        return {Family::NOP, NopZeropageX14, AddressMode::ZeropageX};
    case AslZeropageX:
        return {Family::ASL, AslZeropageX, AddressMode::ZeropageX};
    case ClcImplied:
        return {Family::CLC, ClcImplied, AddressMode::Implied};
    case AslAbsoluteX:
        return {Family::ASL, AslAbsoluteX, AddressMode::AbsoluteX};
    case JsrAbsolute:
        return {Family::JSR, JsrAbsolute, AddressMode::Absolute};
    case AndIndirectX:
        return {Family::AND, AndIndirectX, AddressMode::IndexedIndirect};
    case BitZeropage:
        return {Family::BIT, BitZeropage, AddressMode::Zeropage};
    case AndZeropage:
        return {Family::AND, AndZeropage, AddressMode::Zeropage};
    case RolZeropage:
        return {Family::ROL, RolZeropage, AddressMode::Zeropage};
    case PlpImplied:
        return {Family::PLP, PlpImplied, AddressMode::Implied};
    case AndImmediate:
        return {Family::AND, AndImmediate, AddressMode::Immediate};
    case BitAbsolute:
        return {Family::BIT, BitAbsolute, AddressMode::Absolute};
    case AndAbsolute:
        return {Family::AND, AndAbsolute, AddressMode::Absolute};
    case RolAbsolute:
        return {Family::ROL, RolAbsolute, AddressMode::Absolute};
    case BmiRelative:
        return {Family::BMI, BmiRelative, AddressMode::Relative};
    case AndIndirectY:
        return {Family::AND, AndIndirectY, AddressMode::IndirectIndexed};
    case NopZeropageX34:
        return {Family::NOP, NopZeropageX34, AddressMode::ZeropageX};
    case AndZeropageX:
        return {Family::AND, AndZeropageX, AddressMode::ZeropageX};
    case RolZeropageX:
        return {Family::ROL, RolZeropageX, AddressMode::ZeropageX};
    case SecImplied:
        return {Family::SEC, SecImplied, AddressMode::Implied};
    case AndAbsoluteY:
        return {Family::AND, AndAbsoluteY, AddressMode::AbsoluteY};
    case NopImplied3A:
        return {Family::NOP, NopImplied3A, AddressMode::Implied};
    case AndAbsoluteX:
        return {Family::AND, AndAbsoluteX, AddressMode::AbsoluteX};
    case RolAbsoluteX:
        return {Family::ROL, RolAbsoluteX, AddressMode::AbsoluteX};
    case RtiImplied:
        return {Family::RTI, RtiImplied, AddressMode::Implied};
    case PhaImplied:
        return {Family::PHA, PhaImplied, AddressMode::Implied};
    case LsrAccumulator:
        return {Family::LSR, LsrAccumulator, AddressMode::Accumulator};
    case JmpAbsolute:
        return {Family::JMP, JmpAbsolute, AddressMode::Absolute};
    case BvcRelative:
        return {Family::BVC, BvcRelative, AddressMode::Relative};
    case CliImplied:
        return {Family::CLI, CliImplied, AddressMode::Implied};
    case RtsImplied:
        return {Family::RTS, RtsImplied, AddressMode::Implied};
    case AdcIndirectX:
        return {Family::ADC, AdcIndirectX, AddressMode::IndexedIndirect};
    case NopZeropage64:
        return {Family::NOP, NopZeropage64, AddressMode::Zeropage};
    case AdcZeropage:
        return {Family::ADC, AdcZeropage, AddressMode::Zeropage};
    case RorZeropage:
        return {Family::ROR, RorZeropage, AddressMode::Zeropage};
    case PlaImplied:
        return {Family::PLA, PlaImplied, AddressMode::Implied};
    case AdcImmediate:
        return {Family::ADC, AdcImmediate, AddressMode::Immediate};
    case AdcAbsolute:
        return {Family::ADC, AdcAbsolute, AddressMode::Absolute};
    case RorAbsolute:
        return {Family::ROR, RorAbsolute, AddressMode::Absolute};
    case BvsRelative:
        return {Family::BVS, BvsRelative, AddressMode::Relative};
    case AdcIndirectY:
        return {Family::ADC, AdcIndirectY, AddressMode::IndirectIndexed};
    case NopZeropageX74:
        return {Family::NOP, NopZeropageX74, AddressMode::ZeropageX};
    case AdcZeropageX:
        return {Family::ADC, AdcZeropageX, AddressMode::ZeropageX};
    case RorZeropageX:
        return {Family::ROR, RorZeropageX, AddressMode::ZeropageX};
    case SeiImplied:
        return {Family::SEI, SeiImplied, AddressMode::Implied};
    case AdcAbsoluteY:
        return {Family::ADC, AdcAbsoluteY, AddressMode::AbsoluteY};
    case NopImplied7A:
        return {Family::NOP, NopImplied7A, AddressMode::Implied};
    case AdcAbsoluteX:
        return {Family::ADC, AdcAbsoluteX, AddressMode::AbsoluteX};
    case RorAbsoluteX:
        return {Family::ROR, RorAbsoluteX, AddressMode::AbsoluteX};
    case NopImmediate80:
        return {Family::NOP, NopImmediate80, AddressMode::Immediate};
    case StaIndexedIndirect:
        return {Family::STA, StaIndexedIndirect, AddressMode::IndexedIndirect};
    case StyZeropage:
        return {Family::STY, StyZeropage, AddressMode::Zeropage};
    case StaZeropage:
        return {Family::STA, StaZeropage, AddressMode::Zeropage};
    case StxZeropage:
        return {Family::STX, StxZeropage, AddressMode::Zeropage};
    case DeyImplied:
        return {Family::DEY, DeyImplied, AddressMode::Implied};
    case TxaImplied:
        return {Family::TXA, TxaImplied, AddressMode::Implied};
    case StyAbsolute:
        return {Family::STY, StyAbsolute, AddressMode::Absolute};
    case StaAbsolute:
        return {Family::STA, StaAbsolute, AddressMode::Absolute};
    case StxAbsolute:
        return {Family::STX, StxAbsolute, AddressMode::Absolute};
    case BccRelative:
        return {Family::BCC, BccRelative, AddressMode::Relative};
    case StaIndirectIndexed:
        return {Family::STA, StaIndirectIndexed, AddressMode::IndirectIndexed};
    case StyZeropageX:
        return {Family::STY, StyZeropageX, AddressMode::ZeropageX};
    case StaZeropageX:
        return {Family::STA, StaZeropageX, AddressMode::ZeropageX};
    case StxZeropageY:
        return {Family::STX, StxZeropageY, AddressMode::ZeropageY};
    case TyaImplied:
        return {Family::TYA, TyaImplied, AddressMode::Implied};
    case StaAbsoluteY:
        return {Family::STA, StaAbsoluteY, AddressMode::AbsoluteY};
    case TxsImplied:
        return {Family::TXS, TxsImplied, AddressMode::Implied};
    case StaAbsoluteX:
        return {Family::STA, StaAbsoluteX, AddressMode::AbsoluteX};
    case LdyImmediate:
        return {Family::LDY, LdyImmediate, AddressMode::Immediate};
    case LdaIndirectX:
        return {Family::LDA, LdaIndirectX, AddressMode::IndexedIndirect};
    case LdxImmediate:
        return {Family::LDX, LdxImmediate, AddressMode::Immediate};
    case LaxIndirectX:
        return {Family::LAX, LaxIndirectX, AddressMode::IndexedIndirect};
    case LdyZeropage:
        return {Family::LDY, LdyZeropage, AddressMode::Zeropage};
    case LdaZeropage:
        return {Family::LDA, LdaZeropage, AddressMode::Zeropage};
    case LdxZeropage:
        return {Family::LDX, LdxZeropage, AddressMode::Zeropage};
    case LaxZeropage:
        return {Family::LAX, LaxZeropage, AddressMode::Zeropage};
    case TayImplied:
        return {Family::TAY, TayImplied, AddressMode::Implied};
    case LdaImmediate:
        return {Family::LDA, LdaImmediate, AddressMode::Immediate};
    case TaxImplied:
        return {Family::TAX, TaxImplied, AddressMode::Implied};
    case LdyAbsolute:
        return {Family::LDY, LdyAbsolute, AddressMode::Absolute};
    case LdaAbsolute:
        return {Family::LDA, LdaAbsolute, AddressMode::Absolute};
    case LdxAbsolute:
        return {Family::LDX, LdxAbsolute, AddressMode::Absolute};
    case LaxAbsolute:
        return {Family::LAX, LaxAbsolute, AddressMode::Absolute};
    case BcsRelative:
        return {Family::BCS, BcsRelative, AddressMode::Relative};
    case LdaIndirectY:
        return {Family::LDA, LdaIndirectY, AddressMode::IndirectIndexed};
    case LaxIndirectY:
        return {Family::LAX, LaxIndirectY, AddressMode::IndirectIndexed};
    case LdyZeropageX:
        return {Family::LDY, LdyZeropageX, AddressMode::ZeropageX};
    case LdaZeropageX:
        return {Family::LDA, LdaZeropageX, AddressMode::ZeropageX};
    case LdxZeropageY:
        return {Family::LDX, LdxZeropageY, AddressMode::ZeropageY};
    case LaxZeropageY:
        return {Family::LAX, LaxZeropageY, AddressMode::ZeropageY};
    case ClvImplied:
        return {Family::CLV, ClvImplied, AddressMode::Implied};
    case LdaAbsoluteY:
        return {Family::LDA, LdaAbsoluteY, AddressMode::AbsoluteY};
    case TsxImplied:
        return {Family::TSX, TsxImplied, AddressMode::Implied};
    case LdyAbsoluteX:
        return {Family::LDY, LdyAbsoluteX, AddressMode::AbsoluteX};
    case LdaAbsoluteX:
        return {Family::LDA, LdaAbsoluteX, AddressMode::AbsoluteX};
    case LdxAbsoluteY:
        return {Family::LDX, LdxAbsoluteY, AddressMode::AbsoluteY};
    case LaxAbsoluteY:
        return {Family::LAX, LaxAbsoluteY, AddressMode::AbsoluteY};
    case CpyImmediate:
        return {Family::CPY, CpyImmediate, AddressMode::Immediate};
    case CmpIndirectX:
        return {Family::CMP, CmpIndirectX, AddressMode::IndexedIndirect};
    case CpyZeropage:
        return {Family::CPY, CpyZeropage, AddressMode::Zeropage};
    case DcpIndexedIndirect:
        return {Family::DCP, DcpIndexedIndirect, AddressMode::IndexedIndirect};
    case CmpZeropage:
        return {Family::CMP, CmpZeropage, AddressMode::Zeropage};
    case DecZeropage:
        return {Family::DEC, DecZeropage, AddressMode::Zeropage};
    case DcpZeropage:
        return {Family::DCP, DcpZeropage, AddressMode::Zeropage};
    case InyImplied:
        return {Family::INY, InyImplied, AddressMode::Implied};
    case CmpImmediate:
        return {Family::CMP, CmpImmediate, AddressMode::Immediate};
    case DexImplied:
        return {Family::DEX, DexImplied, AddressMode::Implied};
    case CpyAbsolute:
        return {Family::CPY, CpyAbsolute, AddressMode::Absolute};
    case CmpAbsolute:
        return {Family::CMP, CmpAbsolute, AddressMode::Absolute};
    case DecAbsolute:
        return {Family::DEC, DecAbsolute, AddressMode::Absolute};
    case DcpAbsolute:
        return {Family::DCP, DcpAbsolute, AddressMode::Absolute};
    case BneRelative:
        return {Family::BNE, BneRelative, AddressMode::Relative};
    case CmpIndirectY:
        return {Family::CMP, CmpIndirectY, AddressMode::IndirectIndexed};
    case DcpIndirectIndexed:
        return {Family::DCP, DcpIndirectIndexed, AddressMode::IndirectIndexed};
    case NopZeropageXD4:
        return {Family::NOP, NopZeropageXD4, AddressMode::ZeropageX};
    case CmpZeropageX:
        return {Family::CMP, CmpZeropageX, AddressMode::ZeropageX};
    case DecZeropageX:
        return {Family::DEC, DecZeropageX, AddressMode::ZeropageX};
    case DcpZeropageX:
        return {Family::DCP, DcpZeropageX, AddressMode::ZeropageX};
    case CldImplied:
        return {Family::CLD, CldImplied, AddressMode::Implied};
    case CmpAbsoluteY:
        return {Family::CMP, CmpAbsoluteY, AddressMode::AbsoluteY};
    case NopImpliedDA:
        return {Family::NOP, NopImpliedDA, AddressMode::Implied};
    case DcpAbsoluteY:
        return {Family::DCP, DcpAbsoluteY, AddressMode::AbsoluteY};
    case CmpAbsoluteX:
        return {Family::CMP, CmpAbsoluteX, AddressMode::AbsoluteX};
    case DecAbsoluteX:
        return {Family::DEC, DecAbsoluteX, AddressMode::AbsoluteX};
    case DcpAbsoluteX:
        return {Family::DCP, DcpAbsoluteX, AddressMode::AbsoluteX};
    case CpxImmediate:
        return {Family::CPX, CpxImmediate, AddressMode::Immediate};
    case SbcIndirectX:
        return {Family::SBC, SbcIndirectX, AddressMode::IndexedIndirect};
    case IsbIndexedIndirect:
        return {Family::ISB, IsbIndexedIndirect, AddressMode::IndexedIndirect};
    case CpxZeropage:
        return {Family::CPX, CpxZeropage, AddressMode::Zeropage};
    case SbcZeropage:
        return {Family::SBC, SbcZeropage, AddressMode::Zeropage};
    case IncZeropage:
        return {Family::INC, IncZeropage, AddressMode::Zeropage};
    case IsbZeropage:
        return {Family::ISB, IsbZeropage, AddressMode::Zeropage};
    case InxImplied:
        return {Family::INX, InxImplied, AddressMode::Implied};
    case SbcImmediate:
        return {Family::SBC, SbcImmediate, AddressMode::Immediate};
    case NopImplied:
        return {Family::NOP, NopImplied, AddressMode::Implied};
    case SbcImmediateEB:
        return {Family::SBC, SbcImmediateEB, AddressMode::Immediate};
    case CpxAbsolute:
        return {Family::CPX, CpxAbsolute, AddressMode::Absolute};
    case SbcAbsolute:
        return {Family::SBC, SbcAbsolute, AddressMode::Absolute};
    case IncAbsolute:
        return {Family::INC, IncAbsolute, AddressMode::Absolute};
    case IsbAbsolute:
        return {Family::ISB, IsbAbsolute, AddressMode::Absolute};
    case BeqRelative:
        return {Family::BEQ, BeqRelative, AddressMode::Relative};
    case SbcIndirectY:
        return {Family::SBC, SbcIndirectY, AddressMode::IndirectIndexed};
    case IsbIndirectIndexed:
        return {Family::ISB, IsbIndirectIndexed, AddressMode::IndirectIndexed};
    case NopZeropageXF4:
        return {Family::NOP, NopZeropageXF4, AddressMode::ZeropageX};
    case SbcZeropageX:
        return {Family::SBC, SbcZeropageX, AddressMode::ZeropageX};
    case IncZeropageX:
        return {Family::INC, IncZeropageX, AddressMode::ZeropageX};
    case IsbZeropageX:
        return {Family::ISB, IsbZeropageX, AddressMode::ZeropageX};
    case SedImplied:
        return {Family::SED, SedImplied, AddressMode::Implied};
    case SbcAbsoluteY:
        return {Family::SBC, SbcAbsoluteY, AddressMode::AbsoluteY};
    case NopImpliedFA:
        return {Family::NOP, NopImpliedFA, AddressMode::Implied};
    case IsbAbsoluteY:
        return {Family::ISB, IsbAbsoluteY, AddressMode::AbsoluteY};
    case SbcAbsoluteX:
        return {Family::SBC, SbcAbsoluteX, AddressMode::AbsoluteX};
    case IncAbsoluteX:
        return {Family::INC, IncAbsoluteX, AddressMode::AbsoluteX};
    case EorIndirectX:
        return {Family::EOR, EorIndirectX, AddressMode::IndexedIndirect};
    case NopZeropage44:
        return {Family::NOP, NopZeropage44, AddressMode::Zeropage};
    case EorZeropage:
        return {Family::EOR, EorZeropage, AddressMode::Zeropage};
    case LsrZeropage:
        return {Family::LSR, LsrZeropage, AddressMode::Zeropage};
    case EorImmediate:
        return {Family::EOR, EorImmediate, AddressMode::Immediate};
    case EorAbsolute:
        return {Family::EOR, EorAbsolute, AddressMode::Absolute};
    case LsrAbsolute:
        return {Family::LSR, LsrAbsolute, AddressMode::Absolute};
    case EorIndirectY:
        return {Family::EOR, EorIndirectY, AddressMode::IndirectIndexed};
    case NopZeropageX54:
        return {Family::NOP, NopZeropageX54, AddressMode::ZeropageX};
    case EorZeropageX:
        return {Family::EOR, EorZeropageX, AddressMode::ZeropageX};
    case LsrZeropageX:
        return {Family::LSR, LsrZeropageX, AddressMode::ZeropageX};
    case EorAbsoluteX:
        return {Family::EOR, EorAbsoluteX, AddressMode::AbsoluteX};
    case LsrAbsoluteX:
        return {Family::LSR, LsrAbsoluteX, AddressMode::AbsoluteX};
    case EorAbsoluteY:
        return {Family::EOR, EorAbsoluteY, AddressMode::AbsoluteY};
    case NopImplied5A:
        return {Family::NOP, NopImplied5A, AddressMode::Implied};
    case RolAccumulator:
        return {Family::ROL, RolAccumulator, AddressMode::Accumulator};
    case RorAccumulator:
        return {Family::ROR, RorAccumulator, AddressMode::Accumulator};
    case JmpIndirect:
        return {Family::JMP, JmpIndirect, AddressMode::Indirect};
    case OraImmediate:
        return {Family::ORA, OraImmediate, AddressMode::Immediate};
    case OraAbsolute:
        return {Family::ORA, OraAbsolute, AddressMode::Absolute};
    case OraAbsoluteY:
        return {Family::ORA, OraAbsoluteY, AddressMode::AbsoluteY};
    case NopImplied1A:
        return {Family::NOP, NopImplied1A, AddressMode::Implied};
    case NopAbsoluteX1C:
        return {Family::NOP, NopAbsoluteX1C, AddressMode::AbsoluteX};
    case NopAbsoluteX3C:
        return {Family::NOP, NopAbsoluteX3C, AddressMode::AbsoluteX};
    case NopAbsoluteX5C:
        return {Family::NOP, NopAbsoluteX5C, AddressMode::AbsoluteX};
    case NopAbsoluteX7C:
        return {Family::NOP, NopAbsoluteX7C, AddressMode::AbsoluteX};
    case NopAbsoluteXDC:
        return {Family::NOP, NopAbsoluteXDC, AddressMode::AbsoluteX};
    case NopAbsoluteXFC:
        return {Family::NOP, NopAbsoluteXFC, AddressMode::AbsoluteX};
    case OraAbsoluteX:
        return {Family::ORA, OraAbsoluteX, AddressMode::AbsoluteX};
    case OraZeropage:
        return {Family::ORA, OraZeropage, AddressMode::Zeropage};
    case OraZeropageX:
        return {Family::ORA, OraZeropageX, AddressMode::ZeropageX};
    case OraIndirectIndexed:
        return {Family::ORA, OraIndirectIndexed, AddressMode::IndirectIndexed};
    case SaxIndirectX:
        return {Family::SAX, SaxIndirectX, AddressMode::IndexedIndirect};
    case SaxZeropage:
        return {Family::SAX, SaxZeropage, AddressMode::Zeropage};
    case SaxAbsolute:
        return {Family::SAX, SaxAbsolute, AddressMode::Absolute};
    case SaxZeropageY:
        return {Family::SAX, SaxZeropageY, AddressMode::ZeropageY};
    case IsbAbsoluteX:
        return {Family::ISB, IsbAbsoluteX, AddressMode::AbsoluteX};
    case SloIndexedIndirect:
        return {Family::SLO, SloIndexedIndirect, AddressMode::IndexedIndirect};
    case SloZeropage:
        return {Family::SLO, SloZeropage, AddressMode::Zeropage};
    case SloAbsolute:
        return {Family::SLO, SloAbsolute, AddressMode::Absolute};
    case SloIndirectIndexed:
        return {Family::SLO, SloIndirectIndexed, AddressMode::IndirectIndexed};
    case SloZeropageX:
        return {Family::SLO, SloZeropageX, AddressMode::ZeropageX};
    case SloAbsoluteY:
        return {Family::SLO, SloAbsoluteY, AddressMode::AbsoluteY};
    case SloAbsoluteX:
        return {Family::SLO, SloAbsoluteX, AddressMode::AbsoluteX};
    case RlaIndexedIndirect:
        return {Family::RLA, RlaIndexedIndirect, AddressMode::IndexedIndirect};
    case RlaZeropage:
        return {Family::RLA, RlaZeropage, AddressMode::Zeropage};
    case RlaAbsolute:
        return {Family::RLA, RlaAbsolute, AddressMode::Absolute};
    case RlaIndirectIndexed:
        return {Family::RLA, RlaIndirectIndexed, AddressMode::IndirectIndexed};
    case RlaZeropageX:
        return {Family::RLA, RlaZeropageX, AddressMode::ZeropageX};
    case RlaAbsoluteY:
        return {Family::RLA, RlaAbsoluteY, AddressMode::AbsoluteY};
    case RlaAbsoluteX:
        return {Family::RLA, RlaAbsoluteX, AddressMode::AbsoluteX};
    case SreIndexedIndirect:
        return {Family::SRE, SreIndexedIndirect, AddressMode::IndexedIndirect};
    case SreZeropage:
        return {Family::SRE, SreZeropage, AddressMode::Zeropage};
    case SreAbsolute:
        return {Family::SRE, SreAbsolute, AddressMode::Absolute};
    case SreIndirectIndexed:
        return {Family::SRE, SreIndirectIndexed, AddressMode::IndirectIndexed};
    case SreZeropageX:
        return {Family::SRE, SreZeropageX, AddressMode::ZeropageX};
    case SreAbsoluteY:
        return {Family::SRE, SreAbsoluteY, AddressMode::AbsoluteY};
    case SreAbsoluteX:
        return {Family::SRE, SreAbsoluteX, AddressMode::AbsoluteX};
    case RraIndexedIndirect:
        return {Family::RRA, RraIndexedIndirect, AddressMode::IndexedIndirect};
    case RraZeropage:
        return {Family::RRA, RraZeropage, AddressMode::Zeropage};
    case RraAbsolute:
        return {Family::RRA, RraAbsolute, AddressMode::Absolute};
    case RraIndirectIndexed:
        return {Family::RRA, RraIndirectIndexed, AddressMode::IndirectIndexed};
    case RraZeropageX:
        return {Family::RRA, RraZeropageX, AddressMode::ZeropageX};
    case RraAbsoluteY:
        return {Family::RRA, RraAbsoluteY, AddressMode::AbsoluteY};
    case RraAbsoluteX:
        return {Family::RRA, RraAbsoluteX, AddressMode::AbsoluteX};
    default:
        // Since this is an invalid opcode the instruction and address mode
        // have no real meaning, so we just use 0, 0 for them.
        return {Family::Invalid, BrkImplied, AddressMode::Implied};
    }
}

[[nodiscard]] constexpr MemoryAccess get_memory_access(const Family family) {
    switch (family) {
    // Return Read for instructions where memory access type has no meaning.
    // Set correct access type when we implement missing addressing modes.
    case Family::Invalid:
    case Family::BRK:
    case Family::PHP:
    case Family::BPL:
    case Family::CLC:
    case Family::BIT:
    case Family::PLP:
    case Family::AND:
    case Family::RTI:
    case Family::JSR:
    case Family::BMI:
    case Family::SEC:
    case Family::PHA:
    case Family::JMP:
    case Family::BVC:
    case Family::CLI:
    case Family::ADC:
    case Family::PLA:
    case Family::RTS:
    case Family::BVS:
    case Family::SEI:
    case Family::TXS:
    case Family::BCC:
    case Family::LDX:
    case Family::LDY:
    case Family::LDA:
    case Family::BCS:
    case Family::CLV:
    case Family::BNE:
    case Family::CLD:
    case Family::CPX:
    case Family::NOP:
    case Family::INX:
    case Family::INY:
    case Family::CPY:
    case Family::CMP:
    case Family::BEQ:
    case Family::SED:
    case Family::TYA:
    case Family::TAY:
    case Family::TAX:
    case Family::TSX:
    case Family::TXA:
    case Family::DEY:
    case Family::DEX:
    case Family::EOR:
    case Family::ORA:
    case Family::SBC:
    case Family::LAX:
        return MemoryAccess::Read;
    case Family::STY:
    case Family::STA:
    case Family::STX:
    case Family::SAX:
        return MemoryAccess::Write;
    case Family::INC:
    case Family::DEC:
    case Family::ROL:
    case Family::ROR:
    case Family::ASL:
    case Family::LSR:
    case Family::DCP:
    case Family::ISB:
    case Family::SLO:
    case Family::RLA:
    case Family::SRE:
    case Family::RRA:
        return MemoryAccess::ReadWrite;
    }
    // Should not happen
    throw std::logic_error("Unknown family"); // GCOVR_EXCL_LINE
}

[[nodiscard]] std::string_view to_string(const Family family);

} // namespace n_e_s::core
