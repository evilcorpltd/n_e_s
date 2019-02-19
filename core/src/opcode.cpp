#include "opcode.h"

namespace n_e_s::core {

Opcode decode(const uint8_t op) {
    switch (op) {
    case 0x00:
        return {Instruction::BRK, AddressMode::Implied};
    case 0x08:
        return {Instruction::PHP, AddressMode::Implied};
    case 0x10:
        return {Instruction::BPL, AddressMode::Relative};
    case 0x18:
        return {Instruction::CLC, AddressMode::Implied};
    case 0x20:
        return {Instruction::JSR, AddressMode::Absolute};
    case 0x24:
        return {Instruction::BIT, AddressMode::Zeropage};
    case 0x2C:
        return {Instruction::BIT, AddressMode::Absolute};
    case 0x30:
        return {Instruction::BMI, AddressMode::Relative};
    case 0x38:
        return {Instruction::SEC, AddressMode::Implied};
    case 0x4A:
        return {Instruction::LSR, AddressMode::Accumulator};
    case 0x48:
        return {Instruction::PHA, AddressMode::Implied};
    case 0x4C:
        return {Instruction::JMP, AddressMode::Absolute};
    case 0x50:
        return {Instruction::BVC, AddressMode::Relative};
    case 0x58:
        return {Instruction::CLI, AddressMode::Implied};
    case 0x60:
        return {Instruction::RTS, AddressMode::Implied};
    case 0x65:
        return {Instruction::ADC, AddressMode::Zeropage};
    case 0x69:
        return {Instruction::ADC, AddressMode::Immediate};
    case 0x6D:
        return {Instruction::ADC, AddressMode::Absolute};
    case 0x70:
        return {Instruction::BVS, AddressMode::Relative};
    case 0x78:
        return {Instruction::SEI, AddressMode::Implied};
    case 0x84:
        return {Instruction::STY, AddressMode::Zeropage};
    case 0x85:
        return {Instruction::STA, AddressMode::Zeropage};
    case 0x86:
        return {Instruction::STX, AddressMode::Zeropage};
    case 0x88:
        return {Instruction::DEY, AddressMode::Implied};
    case 0x8A:
        return {Instruction::TXA, AddressMode::Implied};
    case 0x8C:
        return {Instruction::STY, AddressMode::Absolute};
    case 0x8D:
        return {Instruction::STA, AddressMode::Absolute};
    case 0x8E:
        return {Instruction::STX, AddressMode::Absolute};
    case 0x90:
        return {Instruction::BCC, AddressMode::Relative};
    case 0x94:
        return {Instruction::STY, AddressMode::ZeropageX};
    case 0x95:
        return {Instruction::STA, AddressMode::ZeropageX};
    case 0x96:
        return {Instruction::STX, AddressMode::ZeropageY};
    case 0x98:
        return {Instruction::TYA, AddressMode::Implied};
    case 0x99:
        return {Instruction::STA, AddressMode::AbsoluteY};
    case 0x9A:
        return {Instruction::TXS, AddressMode::Implied};
    case 0x9D:
        return {Instruction::STA, AddressMode::AbsoluteX};
    case 0xA0:
        return {Instruction::LDY, AddressMode::Immediate};
    case 0xA8:
        return {Instruction::TAY, AddressMode::Implied};
    case 0xA2:
        return {Instruction::LDX, AddressMode::Immediate};
    case 0xA4:
        return {Instruction::LDY, AddressMode::Zeropage};
    case 0xA5:
        return {Instruction::LDA, AddressMode::Zeropage};
    case 0xA6:
        return {Instruction::LDX, AddressMode::Zeropage};
    case 0xA9:
        return {Instruction::LDA, AddressMode::Immediate};
    case 0xAA:
        return {Instruction::TAX, AddressMode::Implied};
    case 0xAC:
        return {Instruction::LDY, AddressMode::Absolute};
    case 0xAD:
        return {Instruction::LDA, AddressMode::Absolute};
    case 0xAE:
        return {Instruction::LDX, AddressMode::Absolute};
    case 0xB0:
        return {Instruction::BCS, AddressMode::Relative};
    case 0xB4:
        return {Instruction::LDY, AddressMode::ZeropageX};
    case 0xB5:
        return {Instruction::LDA, AddressMode::ZeropageX};
    case 0xB6:
        return {Instruction::LDX, AddressMode::ZeropageY};
    case 0xB8:
        return {Instruction::CLV, AddressMode::Implied};
    case 0xBA:
        return {Instruction::TSX, AddressMode::Implied};
    case 0xC8:
        return {Instruction::INY, AddressMode::Implied};
    case 0xC0:
        return {Instruction::CPY, AddressMode::Immediate};
    case 0xC4:
        return {Instruction::CPY, AddressMode::Zeropage};
    case 0xCA:
        return {Instruction::DEX, AddressMode::Implied};
    case 0xCC:
        return {Instruction::CPY, AddressMode::Absolute};
    case 0xD0:
        return {Instruction::BNE, AddressMode::Relative};
    case 0xD8:
        return {Instruction::CLD, AddressMode::Implied};
    case 0xE0:
        return {Instruction::CPX, AddressMode::Immediate};
    case 0xE4:
        return {Instruction::CPX, AddressMode::Zeropage};
    case 0xE8:
        return {Instruction::INX, AddressMode::Implied};
    case 0xEA:
        return {Instruction::NOP, AddressMode::Implied};
    case 0xEC:
        return {Instruction::CPX, AddressMode::Absolute};
    case 0xF0:
        return {Instruction::BEQ, AddressMode::Relative};
    case 0xF8:
        return {Instruction::SED, AddressMode::Implied};
    }

    return {Instruction::Invalid, AddressMode::Invalid};
}

} // namespace n_e_s::core
