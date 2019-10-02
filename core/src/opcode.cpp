#include "core/opcode.h"

#include <string>

namespace n_e_s::core {

Opcode decode(const uint8_t op) {
    switch (op) {
    case BrkImplied:
        return {Family::BRK, BrkImplied, AddressMode::Implied};
    case PhpImplied:
        return {Family::PHP, PhpImplied, AddressMode::Implied};
    case BplRelative:
        return {Family::BPL, BplRelative, AddressMode::Relative};
    case ClcImplied:
        return {Family::CLC, ClcImplied, AddressMode::Implied};
    case JsrAbsolute:
        return {Family::JSR, JsrAbsolute, AddressMode::Absolute};
    case BitZeropage:
        return {Family::BIT, BitZeropage, AddressMode::Zeropage};
    case PlpImplied:
        return {Family::BIT, PlpImplied, AddressMode::Implied};
    case AndImmediate:
        return {Family::AND, AndImmediate, AddressMode::Immediate};
    case BitAbsolute:
        return {Family::BIT, BitAbsolute, AddressMode::Absolute};
    case AndAbsolute:
        return {Family::AND, AndAbsolute, AddressMode::Absolute};
    case BmiRelative:
        return {Family::BMI, BmiRelative, AddressMode::Relative};
    case SecImplied:
        return {Family::SEC, SecImplied, AddressMode::Implied};
    case AndAbsoluteY:
        return {Family::AND, AndAbsoluteY, AddressMode::AbsoluteY};
    case AndAbsoluteX:
        return {Family::AND, AndAbsoluteX, AddressMode::AbsoluteX};
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
    case AdcZeropage:
        return {Family::ADC, AdcZeropage, AddressMode::Zeropage};
    case PlaImplied:
        return {Family::PLA, PlaImplied, AddressMode::Implied};
    case AdcImmediate:
        return {Family::ADC, AdcImmediate, AddressMode::Immediate};
    case AdcAbsolute:
        return {Family::ADC, AdcAbsolute, AddressMode::Absolute};
    case BvsRelative:
        return {Family::BVS, BvsRelative, AddressMode::Relative};
    case SeiImplied:
        return {Family::SEI, SeiImplied, AddressMode::Implied};
    case AdcAbsoluteY:
        return {Family::ADC, AdcAbsoluteY, AddressMode::AbsoluteY};
    case AdcAbsoluteX:
        return {Family::ADC, AdcAbsoluteX, AddressMode::AbsoluteX};
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
    case LdxImmediate:
        return {Family::LDX, LdxImmediate, AddressMode::Immediate};
    case LdyZeropage:
        return {Family::LDY, LdyZeropage, AddressMode::Zeropage};
    case LdaZeropage:
        return {Family::LDA, LdaZeropage, AddressMode::Zeropage};
    case LdxZeropage:
        return {Family::LDX, LdxZeropage, AddressMode::Zeropage};
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
    case BcsRelative:
        return {Family::BCS, BcsRelative, AddressMode::Relative};
    case LdyZeropageX:
        return {Family::LDY, LdyZeropageX, AddressMode::ZeropageX};
    case LdaZeropageX:
        return {Family::LDA, LdaZeropageX, AddressMode::ZeropageX};
    case LdxZeropageY:
        return {Family::LDX, LdxZeropageY, AddressMode::ZeropageY};
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
    case CpyImmediate:
        return {Family::CPY, CpyImmediate, AddressMode::Immediate};
    case CmpIndirectX:
        return {Family::CMP, CmpIndirectX, AddressMode::IndexedIndirect};
    case CpyZeropage:
        return {Family::CPY, CpyZeropage, AddressMode::Zeropage};
    case CmpZeropage:
        return {Family::CMP, CmpZeropage, AddressMode::Zeropage};
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
    case BneRelative:
        return {Family::BNE, BneRelative, AddressMode::Relative};
    case CmpIndirectY:
        return {Family::CMP, CmpIndirectY, AddressMode::IndirectIndexed};
    case CmpZeropageX:
        return {Family::CMP, CmpZeropageX, AddressMode::ZeropageX};
    case CldImplied:
        return {Family::CLD, CldImplied, AddressMode::Implied};
    case CmpAbsoluteY:
        return {Family::CMP, CmpAbsoluteY, AddressMode::AbsoluteY};
    case CmpAbsoluteX:
        return {Family::CMP, CmpAbsoluteX, AddressMode::AbsoluteX};
    case CpxImmediate:
        return {Family::CPX, CpxImmediate, AddressMode::Immediate};
    case CpxZeropage:
        return {Family::CPX, CpxZeropage, AddressMode::Zeropage};
    case IncZeropage:
        return {Family::INC, IncZeropage, AddressMode::Zeropage};
    case InxImplied:
        return {Family::INX, InxImplied, AddressMode::Implied};
    case NopImplied:
        return {Family::NOP, NopImplied, AddressMode::Implied};
    case CpxAbsolute:
        return {Family::CPX, CpxAbsolute, AddressMode::Absolute};
    case BeqRelative:
        return {Family::BEQ, BeqRelative, AddressMode::Relative};
    case SedImplied:
        return {Family::SED, SedImplied, AddressMode::Implied};
    default:
        // Since this is an invalid opcode the instruction and address mode
        // have no real meaning, so we just use 0, 0 for them.
        return {Family::Invalid, BrkImplied, AddressMode::Implied};
    }
}

std::string to_string(const Family family) {
    switch (family) {
    case Family::Invalid:
        return "Invalid";
    case Family::BRK:
        return "BRK";
    case Family::INC:
        return "INC";
    case Family::PHP:
        return "PHP";
    case Family::BPL:
        return "BPL";
    case Family::CLC:
        return "CLC";
    case Family::BIT:
        return "BIT";
    case Family::PLP:
        return "PLP";
    case Family::AND:
        return "AND";
    case Family::JSR:
        return "JSR";
    case Family::BMI:
        return "BMI";
    case Family::SEC:
        return "SEC";
    case Family::LSR:
        return "LSR";
    case Family::PHA:
        return "PHA";
    case Family::JMP:
        return "JMP";
    case Family::BVC:
        return "BVC";
    case Family::CLI:
        return "CLI";
    case Family::ADC:
        return "ADC";
    case Family::PLA:
        return "PLA";
    case Family::RTS:
        return "RTS";
    case Family::BVS:
        return "BVS";
    case Family::SEI:
        return "SEI";
    case Family::STY:
        return "STY";
    case Family::STA:
        return "STA";
    case Family::STX:
        return "STX";
    case Family::TXS:
        return "TXS";
    case Family::BCC:
        return "BCC";
    case Family::LDX:
        return "LDX";
    case Family::LDY:
        return "LDY";
    case Family::LDA:
        return "LDA";
    case Family::BCS:
        return "BCS";
    case Family::CLV:
        return "CLV";
    case Family::BNE:
        return "BNE";
    case Family::CLD:
        return "CLD";
    case Family::CPX:
        return "CPX";
    case Family::NOP:
        return "NOP";
    case Family::INX:
        return "INX";
    case Family::INY:
        return "INY";
    case Family::CPY:
        return "CPY";
    case Family::CMP:
        return "CMP";
    case Family::BEQ:
        return "BEQ";
    case Family::SED:
        return "SED";
    case Family::TYA:
        return "TYA";
    case Family::TAY:
        return "TAY";
    case Family::TAX:
        return "TAX";
    case Family::TSX:
        return "TSX";
    case Family::TXA:
        return "TXA";
    case Family::DEY:
        return "DEY";
    case Family::DEX:
        return "DEX";
    default:
        return "UNKNOWN";
    }
}

} // namespace n_e_s::core
