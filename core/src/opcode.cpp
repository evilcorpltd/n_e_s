#include "opcode.h"

namespace n_e_s::core {

Opcode decode(const uint8_t op) {
    switch (op) {
    case BRK_implied:
        return {Family::BRK, BRK_implied, AddressMode::Implied};
    case PHP_implied:
        return {Family::PHP, PHP_implied, AddressMode::Implied};
    case BPL_relative:
        return {Family::BPL, BPL_relative, AddressMode::Relative};
    case CLC_implied:
        return {Family::CLC, CLC_implied, AddressMode::Implied};
    case JSR_absolute:
        return {Family::JSR, JSR_absolute, AddressMode::Absolute};
    case BIT_zeropage:
        return {Family::BIT, BIT_zeropage, AddressMode::Zeropage};
    case BIT_absolute:
        return {Family::BIT, BIT_absolute, AddressMode::Absolute};
    case BMI_relative:
        return {Family::BMI, BMI_relative, AddressMode::Relative};
    case SEC_implied:
        return {Family::SEC, SEC_implied, AddressMode::Implied};
    case PHA_implied:
        return {Family::PHA, PHA_implied, AddressMode::Implied};
    case LSR_accumulator:
        return {Family::LSR, LSR_accumulator, AddressMode::Accumulator};
    case JMP_absolute:
        return {Family::JMP, JMP_absolute, AddressMode::Absolute};
    case BVC_relative:
        return {Family::BVC, BVC_relative, AddressMode::Relative};
    case CLI_implied:
        return {Family::CLI, CLI_implied, AddressMode::Implied};
    case RTS_implied:
        return {Family::RTS, RTS_implied, AddressMode::Implied};
    case ADC_zeropage:
        return {Family::ADC, ADC_zeropage, AddressMode::Zeropage};
    case ADC_immediate:
        return {Family::ADC, ADC_immediate, AddressMode::Immediate};
    case ADC_absolute:
        return {Family::ADC, ADC_absolute, AddressMode::Absolute};
    case BVS_relative:
        return {Family::BVS, BVS_relative, AddressMode::Relative};
    case SEI_implied:
        return {Family::SEI, SEI_implied, AddressMode::Implied};
    case STA_indexed_indirect:
        return {Family::STA,
                STA_indexed_indirect,
                AddressMode::IndexedIndirect};
    case STY_zeropage:
        return {Family::STY, STY_zeropage, AddressMode::Zeropage};
    case STA_zeropage:
        return {Family::STA, STA_zeropage, AddressMode::Zeropage};
    case STX_zeropage:
        return {Family::STX, STX_zeropage, AddressMode::Zeropage};
    case DEY_implied:
        return {Family::DEY, DEY_implied, AddressMode::Implied};
    case TXA_implied:
        return {Family::TXA, TXA_implied, AddressMode::Implied};
    case STY_absolute:
        return {Family::STY, STY_absolute, AddressMode::Absolute};
    case STA_absolute:
        return {Family::STA, STA_absolute, AddressMode::Absolute};
    case STX_absolute:
        return {Family::STX, STX_absolute, AddressMode::Absolute};
    case BCC_relative:
        return {Family::BCC, BCC_relative, AddressMode::Relative};
    case STA_indirect_indexed:
        return {Family::STA,
                STA_indirect_indexed,
                AddressMode::IndirectIndexed};
    case STY_zeropageX:
        return {Family::STY, STY_zeropageX, AddressMode::ZeropageX};
    case STA_zeropageX:
        return {Family::STA, STA_zeropageX, AddressMode::ZeropageX};
    case STX_zeropageY:
        return {Family::STX, STX_zeropageY, AddressMode::ZeropageY};
    case TYA_implied:
        return {Family::TYA, TYA_implied, AddressMode::Implied};
    case STA_absoluteY:
        return {Family::STA, STA_absoluteY, AddressMode::AbsoluteY};
    case TXS_implied:
        return {Family::TXS, TXS_implied, AddressMode::Implied};
    case STA_absoluteX:
        return {Family::STA, STA_absoluteX, AddressMode::AbsoluteX};
    case LDY_immediate:
        return {Family::LDY, LDY_immediate, AddressMode::Immediate};
    case LDX_immediate:
        return {Family::LDX, LDX_immediate, AddressMode::Immediate};
    case LDY_zeropage:
        return {Family::LDY, LDY_zeropage, AddressMode::Zeropage};
    case LDA_zeropage:
        return {Family::LDA, LDA_zeropage, AddressMode::Zeropage};
    case LDX_zeropage:
        return {Family::LDX, LDX_zeropage, AddressMode::Zeropage};
    case TAY_implied:
        return {Family::TAY, TAY_implied, AddressMode::Implied};
    case LDA_immediate:
        return {Family::LDA, LDA_immediate, AddressMode::Immediate};
    case TAX_implied:
        return {Family::TAX, TAX_implied, AddressMode::Implied};
    case LDY_absolute:
        return {Family::LDY, LDY_absolute, AddressMode::Absolute};
    case LDA_absolute:
        return {Family::LDA, LDA_absolute, AddressMode::Absolute};
    case LDX_absolute:
        return {Family::LDX, LDX_absolute, AddressMode::Absolute};
    case BCS_relative:
        return {Family::BCS, BCS_relative, AddressMode::Relative};
    case LDY_zeropageX:
        return {Family::LDY, LDY_zeropageX, AddressMode::ZeropageX};
    case LDA_zeropageX:
        return {Family::LDA, LDA_zeropageX, AddressMode::ZeropageX};
    case LDX_zeropageY:
        return {Family::LDX, LDX_zeropageY, AddressMode::ZeropageY};
    case CLV_implied:
        return {Family::CLV, CLV_implied, AddressMode::Implied};
    case TSX_implied:
        return {Family::TSX, TSX_implied, AddressMode::Implied};
    case CPY_immediate:
        return {Family::CPY, CPY_immediate, AddressMode::Immediate};
    case CPY_zeropage:
        return {Family::CPY, CPY_zeropage, AddressMode::Zeropage};
    case INY_implied:
        return {Family::INY, INY_implied, AddressMode::Implied};
    case DEX_implied:
        return {Family::DEX, DEX_implied, AddressMode::Implied};
    case CPY_absolute:
        return {Family::CPY, CPY_absolute, AddressMode::Absolute};
    case BNE_relative:
        return {Family::BNE, BNE_relative, AddressMode::Relative};
    case CLD_implied:
        return {Family::CLD, CLD_implied, AddressMode::Implied};
    case CPX_immediate:
        return {Family::CPX, CPX_immediate, AddressMode::Immediate};
    case CPX_zeropage:
        return {Family::CPX, CPX_zeropage, AddressMode::Zeropage};
    case INX_implied:
        return {Family::INX, INX_implied, AddressMode::Implied};
    case NOP_implied:
        return {Family::NOP, NOP_implied, AddressMode::Implied};
    case CPX_absolute:
        return {Family::CPX, CPX_absolute, AddressMode::Absolute};
    case BEQ_relative:
        return {Family::BEQ, BEQ_relative, AddressMode::Relative};
    case SED_implied:
        return {Family::SED, SED_implied, AddressMode::Implied};
    default:
        // Since this is an invalid opcode the instruction and address mode
        // have no real meaning, so we just use 0, 0 for them.
        return {Family::Invalid, BRK_implied, AddressMode::Implied};
    }
}

} // namespace n_e_s::core
