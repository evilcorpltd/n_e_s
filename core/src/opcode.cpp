#include "core/opcode.h"

#include <stdexcept>
#include <string>

namespace n_e_s::core {

Opcode decode(const uint8_t op) {
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
    case NopAbsolute0c:
        return {Family::NOP, NopAbsolute0c, AddressMode::Absolute};
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
    case NopImplied3a:
        return {Family::NOP, NopImplied3a, AddressMode::Implied};
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
    case NopImplied7a:
        return {Family::NOP, NopImplied7a, AddressMode::Implied};
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
    case LdaIndirectY:
        return {Family::LDA, LdaIndirectY, AddressMode::IndirectIndexed};
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
    case DecZeropage:
        return {Family::DEC, DecZeropage, AddressMode::Zeropage};
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
    case BneRelative:
        return {Family::BNE, BneRelative, AddressMode::Relative};
    case CmpIndirectY:
        return {Family::CMP, CmpIndirectY, AddressMode::IndirectIndexed};
    case NopZeropageXd4:
        return {Family::NOP, NopZeropageXd4, AddressMode::ZeropageX};
    case CmpZeropageX:
        return {Family::CMP, CmpZeropageX, AddressMode::ZeropageX};
    case DecZeropageX:
        return {Family::DEC, DecZeropageX, AddressMode::ZeropageX};
    case CldImplied:
        return {Family::CLD, CldImplied, AddressMode::Implied};
    case CmpAbsoluteY:
        return {Family::CMP, CmpAbsoluteY, AddressMode::AbsoluteY};
    case NopImpliedDA:
        return {Family::NOP, NopImpliedDA, AddressMode::Implied};
    case CmpAbsoluteX:
        return {Family::CMP, CmpAbsoluteX, AddressMode::AbsoluteX};
    case DecAbsoluteX:
        return {Family::DEC, DecAbsoluteX, AddressMode::AbsoluteX};
    case CpxImmediate:
        return {Family::CPX, CpxImmediate, AddressMode::Immediate};
    case SbcIndirectX:
        return {Family::SBC, SbcIndirectX, AddressMode::IndexedIndirect};
    case CpxZeropage:
        return {Family::CPX, CpxZeropage, AddressMode::Zeropage};
    case SbcZeropage:
        return {Family::SBC, SbcZeropage, AddressMode::Zeropage};
    case IncZeropage:
        return {Family::INC, IncZeropage, AddressMode::Zeropage};
    case InxImplied:
        return {Family::INX, InxImplied, AddressMode::Implied};
    case SbcImmediate:
        return {Family::SBC, SbcImmediate, AddressMode::Immediate};
    case NopImplied:
        return {Family::NOP, NopImplied, AddressMode::Implied};
    case CpxAbsolute:
        return {Family::CPX, CpxAbsolute, AddressMode::Absolute};
    case SbcAbsolute:
        return {Family::SBC, SbcAbsolute, AddressMode::Absolute};
    case IncAbsolute:
        return {Family::INC, IncAbsolute, AddressMode::Absolute};
    case BeqRelative:
        return {Family::BEQ, BeqRelative, AddressMode::Relative};
    case SbcIndirectY:
        return {Family::SBC, SbcIndirectY, AddressMode::IndirectIndexed};
    case NopZeropageXf4:
        return {Family::NOP, NopZeropageXf4, AddressMode::ZeropageX};
    case SbcZeropageX:
        return {Family::SBC, SbcZeropageX, AddressMode::ZeropageX};
    case IncZeropageX:
        return {Family::INC, IncZeropageX, AddressMode::ZeropageX};
    case SedImplied:
        return {Family::SED, SedImplied, AddressMode::Implied};
    case SbcAbsoluteY:
        return {Family::SBC, SbcAbsoluteY, AddressMode::AbsoluteY};
    case NopImpliedFA:
        return {Family::NOP, NopImpliedFA, AddressMode::Implied};
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
    case NopImplied5a:
        return {Family::NOP, NopImplied5a, AddressMode::Implied};
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
    case NopImplied1a:
        return {Family::NOP, NopImplied1a, AddressMode::Implied};
    case OraAbsoluteX:
        return {Family::ORA, OraAbsoluteX, AddressMode::AbsoluteX};
    case OraZeropage:
        return {Family::ORA, OraZeropage, AddressMode::Zeropage};
    case OraZeropageX:
        return {Family::ORA, OraZeropageX, AddressMode::ZeropageX};
    case OraIndirectIndexed:
        return {Family::ORA, OraIndirectIndexed, AddressMode::IndirectIndexed};
    default:
        // Since this is an invalid opcode the instruction and address mode
        // have no real meaning, so we just use 0, 0 for them.
        return {Family::Invalid, BrkImplied, AddressMode::Implied};
    }
}

MemoryAccess get_memory_access(const Family family) {
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
        return MemoryAccess::Read;
    case Family::STY:
    case Family::STA:
    case Family::STX:
        return MemoryAccess::Write;
    case Family::INC:
    case Family::DEC:
    case Family::ROL:
    case Family::ROR:
    case Family::ASL:
    case Family::LSR:
        return MemoryAccess::ReadWrite;
    }
    // Should not happen
    throw std::logic_error("Unknown family");
}

std::string_view to_string(const Family family) {
    switch (family) {
    case Family::Invalid:
        return "Invalid";
    case Family::BRK:
        return "BRK";
    case Family::ASL:
        return "ASL";
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
    case Family::RTI:
        return "RTI";
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
    case Family::DEC:
        return "DEC";
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
    case Family::EOR:
        return "EOR";
    case Family::ROL:
        return "ROL";
    case Family::ROR:
        return "ROR";
    case Family::ORA:
        return "ORA";
    case Family::SBC:
        return "SBC";
    }

    // Should not happen
    throw std::logic_error("Unknown family");
}

} // namespace n_e_s::core
