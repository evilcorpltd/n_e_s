#include "mos6502.h"

#include "opcode.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace {

const uint16_t kResetAddress = 0xFFFC; // This is where the reset routine is.
const uint16_t kBrkAddress = 0xFFFE; // This is where the break routine is.

constexpr bool is_negative(uint8_t byte) {
    return byte & (1 << 7);
}

constexpr uint8_t low_bits(uint8_t byte) {
    return byte & ~(1 << 7);
}

constexpr int8_t to_signed(uint8_t byte) {
    if (is_negative(byte)) {
        return low_bits(byte) - 128;
    }

    return low_bits(byte);
}

constexpr uint16_t high_byte(uint16_t word) {
    return word & 0xFF00;
}

} // namespace

namespace n_e_s::core {

Mos6502::Stack::Stack(Registers *registers, IMmu *mmu)
        : registers_(registers), mmu_(mmu) {}

uint8_t Mos6502::Stack::pop_byte() {
    return mmu_->read_byte(ram_offset_ + ++registers_->sp);
}

uint16_t Mos6502::Stack::pop_word() {
    const uint16_t ret = mmu_->read_word(ram_offset_ + ++registers_->sp);
    ++registers_->sp;
    return ret;
}

void Mos6502::Stack::push_byte(uint8_t byte) {
    mmu_->write_byte(ram_offset_ + registers_->sp--, byte);
}

void Mos6502::Stack::push_word(uint16_t word) {
    mmu_->write_word(ram_offset_ + --registers_->sp, word);
    --registers_->sp;
}

Mos6502::Mos6502(Registers *const registers, IMmu *const mmu)
        : registers_(registers),
          mmu_(mmu),
          stack_(registers_, mmu_),
          pipeline_() {}

void Mos6502::execute() {
    if (pipeline_.done()) {
        pipeline_ = parse_next_instruction();
    } else {
        pipeline_.execute_step();
    }
}

// Most instruction timings are from https://robinli.eu/f/6502_cpu.txt
Pipeline Mos6502::parse_next_instruction() {
    Pipeline result;
    const uint8_t raw_opcode{mmu_->read_byte(registers_->pc++)};
    const Opcode opcode = decode(raw_opcode);

    if (opcode.family == Family::Invalid) {
        std::stringstream err;
        err << "Bad instruction: " << std::showbase << std::hex << +raw_opcode;
        throw std::logic_error(err.str());
    }

    if (opcode.address_mode == AddressMode::Immediate) {
        effective_address_ = registers_->pc++;
    }

    switch (opcode.instruction) {
    case Instruction::BRK_implied:
        result.push([=]() { ++registers_->pc; });
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() { stack_.push_word(registers_->pc); });
        result.push([=]() { stack_.push_byte(registers_->p | B_FLAG); });
        result.push([=]() { ++registers_->pc; });
        result.push([=]() { registers_->pc = mmu_->read_word(kBrkAddress); });
        break;
    case Instruction::PHP_implied:
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() { stack_.push_byte(registers_->p); });
        break;
    case Instruction::BPL_relative:
        result.append(create_branch_instruction(
                [=]() { return !(registers_->p & N_FLAG); }));
        break;
    case Instruction::BIT_zeropage:
    case Instruction::BIT_absolute:
        if (opcode.address_mode == AddressMode::Absolute) {
            result.append(create_absolute_addressing_steps());
        } else if (opcode.address_mode == AddressMode::Zeropage) {
            result.append(create_zeropage_addressing_steps());
        }

        result.push([=]() {
            const uint8_t value = mmu_->read_byte(effective_address_);
            set_zero(value & registers_->a);
            set_negative(value);
            if (value & (1u << 6)) {
                set_flag(V_FLAG);
            } else {
                clear_flag(V_FLAG);
            }
        });
        break;
    case Instruction::CLC_implied:
        result.push([=]() { clear_flag(C_FLAG); });
        break;
    case Instruction::JSR_absolute:
        result.append(create_absolute_addressing_steps());
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() { stack_.push_word(--registers_->pc); });
        result.push([=]() { registers_->pc = effective_address_; });
        break;
    case Instruction::BMI_relative:
        result.append(create_branch_instruction(
                [=]() { return registers_->p & N_FLAG; }));
        break;
    case Instruction::SEC_implied:
        result.push([=]() { set_flag(C_FLAG); });
        break;
    case Instruction::LSR_accumulator:
        if (opcode.address_mode == AddressMode::Accumulator) {
            result.push([=]() {
                set_carry(registers_->a & 1);
                registers_->a &= ~1;
                registers_->a >>= 1;
                set_zero(registers_->a);
                clear_flag(N_FLAG);
            });
        }
        break;
    case Instruction::PHA_implied:
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() { stack_.push_byte(registers_->a); });
        break;
    case Instruction::JMP_absolute:
        result.push([=]() { ++registers_->pc; });
        result.push([=]() {
            registers_->pc = mmu_->read_word(registers_->pc - 1);
        });
        break;
    case Instruction::BVC_relative:
        result.append(create_branch_instruction(
                [=]() { return !(registers_->p & V_FLAG); }));
        break;
    case Instruction::CLI_implied:
        result.push([=]() { clear_flag(I_FLAG); });
        break;
    case Instruction::ADC_zeropage:
    case Instruction::ADC_immediate:
    case Instruction::ADC_absolute:
        result.append(create_add_instruction(opcode));
        break;
    case Instruction::RTS_implied:
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() {
            /* Do nothing. */
        });
        result.push([=]() { registers_->pc = stack_.pop_word(); });
        result.push([=]() { ++registers_->pc; });
        break;
    case Instruction::BVS_relative:
        result.append(create_branch_instruction(
                [=]() { return registers_->p & V_FLAG; }));
        break;
    case Instruction::SEI_implied:
        result.push([=]() { set_flag(I_FLAG); });
        break;
    case Instruction::STA_indexed_indirect:
    case Instruction::STA_zeropage:
    case Instruction::STA_absolute:
    case Instruction::STA_indirect_indexed:
    case Instruction::STA_zeropageX:
    case Instruction::STA_absoluteY:
    case Instruction::STA_absoluteX:
    case Instruction::STX_zeropage:
    case Instruction::STX_absolute:
    case Instruction::STX_zeropageY:
    case Instruction::STY_zeropage:
    case Instruction::STY_absolute:
    case Instruction::STY_zeropageX:
        result.append(create_store_instruction(opcode));
        break;
    case Instruction::TXS_implied:
        result.push([=]() { registers_->sp = registers_->x; });
        break;
    case Instruction::TYA_implied:
        result.push([=]() {
            registers_->a = registers_->y;
            set_zero(registers_->a);
            set_negative(registers_->a);
        });
        break;
    case Instruction::TAY_implied:
        result.push([=]() {
            registers_->y = registers_->a;
            set_zero(registers_->y);
            set_negative(registers_->y);
        });
        break;
    case Instruction::TAX_implied:
        result.push([=]() {
            registers_->x = registers_->a;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::TSX_implied:
        result.push([=]() {
            registers_->x = registers_->sp;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::TXA_implied:
        result.push([=]() {
            registers_->a = registers_->x;
            set_zero(registers_->a);
            set_negative(registers_->a);
        });
        break;
    case Instruction::BCC_relative:
        result.append(create_branch_instruction(
                [=]() { return !(registers_->p & C_FLAG); }));
        break;
    case Instruction::LDA_zeropage:
    case Instruction::LDA_immediate:
    case Instruction::LDA_absolute:
    case Instruction::LDA_zeropageX:
    case Instruction::LDX_immediate:
    case Instruction::LDX_zeropage:
    case Instruction::LDX_absolute:
    case Instruction::LDX_zeropageY:
    case Instruction::LDY_immediate:
    case Instruction::LDY_zeropage:
    case Instruction::LDY_absolute:
    case Instruction::LDY_zeropageX:
        result.append(create_load_instruction(opcode));
        break;
    case Instruction::BCS_relative:
        result.append(create_branch_instruction(
                [=]() { return registers_->p & C_FLAG; }));
        break;
    case Instruction::CLV_implied:
        result.push([=]() { clear_flag(V_FLAG); });
        break;
    case Instruction::BNE_relative:
        result.append(create_branch_instruction(
                [=]() { return !(registers_->p & Z_FLAG); }));
        break;
    case Instruction::CLD_implied:
        result.push([=]() { clear_flag(D_FLAG); });
        break;
    case Instruction::CPX_immediate:
    case Instruction::CPX_zeropage:
    case Instruction::CPX_absolute:
    case Instruction::CPY_immediate:
    case Instruction::CPY_zeropage:
    case Instruction::CPY_absolute:
        result.append(create_compare_instruction(opcode));
        break;
    case Instruction::NOP_implied:
        result.push([]() { /* Do nothing. */ });
        break;
    case Instruction::INX_implied:
        result.push([=]() {
            ++registers_->x;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::DEX_implied:
        result.push([=]() {
            --registers_->x;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::INY_implied:
        result.push([=]() {
            ++registers_->y;
            set_zero(registers_->y);
            set_negative(registers_->y);
        });
        break;
    case Instruction::DEY_implied:
        result.push([=]() {
            --registers_->y;
            set_zero(registers_->y);
            set_negative(registers_->y);
        });
        break;
    case Instruction::BEQ_relative:
        result.append(create_branch_instruction(
                [=]() { return registers_->p & Z_FLAG; }));
        break;
    case Instruction::SED_implied:
        result.push([=]() { set_flag(D_FLAG); });
        break;
    }
    return result;
} // namespace n_e_s::core

void Mos6502::reset() {
    pipeline_.clear();

    registers_->pc = mmu_->read_word(kResetAddress);
}

void Mos6502::clear_flag(uint8_t flag) {
    registers_->p &= ~flag;
}

void Mos6502::set_flag(uint8_t flag) {
    registers_->p |= flag;
}

void Mos6502::set_carry(bool carry) {
    if (carry) {
        set_flag(C_FLAG);
    } else {
        clear_flag(C_FLAG);
    }
}

void Mos6502::set_zero(uint8_t byte) {
    if (byte == 0) {
        set_flag(Z_FLAG);
    } else {
        clear_flag(Z_FLAG);
    }
}

void Mos6502::set_negative(uint8_t byte) {
    if (is_negative(byte)) {
        set_flag(N_FLAG);
    } else {
        clear_flag(N_FLAG);
    }
}

void Mos6502::set_overflow(uint8_t reg_value,
        uint8_t operand,
        uint16_t resulting_value) {
    // See: http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    const bool overflow = ((reg_value ^ resulting_value) &
                                  (operand ^ resulting_value) & 0x80) != 0;
    if (overflow) {
        set_flag(V_FLAG);
    } else {
        clear_flag(V_FLAG);
    }
}

Pipeline Mos6502::create_branch_instruction(
        const std::function<bool()> &condition) {
    Pipeline result;

    result.push_conditional([=]() {
        if (!condition()) {
            ++registers_->pc;
            return false;
        }
        return true;
    });

    result.push_conditional([=]() {
        const uint8_t offset = mmu_->read_byte(registers_->pc++);
        const uint16_t page = high_byte(registers_->pc);

        registers_->pc += to_signed(offset);

        if (page != high_byte(registers_->pc)) {
            return true;
            // We crossed a page boundary so we spend 1 more cycle.
        }
        return false;
    });

    result.push([=]() { /* Do nothing. */ });

    return result;
}

Pipeline Mos6502::create_add_instruction(Opcode opcode) {
    Pipeline result;
    if (opcode.address_mode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.address_mode == AddressMode::Zeropage) {
        result.append(create_zeropage_addressing_steps());
    }

    result.push([=]() {
        const uint8_t a_before = registers_->a;
        const uint8_t addend = mmu_->read_byte(effective_address_);
        const uint8_t carry = registers_->p & C_FLAG ? 1u : 0u;
        const uint16_t temp_result = registers_->a + addend + carry;
        registers_->a = static_cast<uint8_t>(temp_result);

        set_carry(temp_result > 0xFF);
        set_zero(registers_->a);
        set_negative(registers_->a);
        set_overflow(a_before, addend, temp_result);
    });

    return result;
}

Pipeline Mos6502::create_store_instruction(Opcode opcode) {
    Pipeline result;
    if (opcode.address_mode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.address_mode == AddressMode::AbsoluteX) {
        result.append(create_absolute_indexed_addressing_steps(&registers_->x));
    } else if (opcode.address_mode == AddressMode::AbsoluteY) {
        result.append(create_absolute_indexed_addressing_steps(&registers_->y));
    } else if (opcode.address_mode == AddressMode::Zeropage) {
        result.append(create_zeropage_addressing_steps());
    } else if (opcode.address_mode == AddressMode::ZeropageX) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->x));
    } else if (opcode.address_mode == AddressMode::ZeropageY) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->y));
    } else if (opcode.address_mode == AddressMode::IndexedIndirect) {
        result.append(create_indexed_indirect_addressing_steps());
    } else if (opcode.address_mode == AddressMode::IndirectIndexed) {
        result.append(create_indirect_indexed_addressing_steps());
    }

    uint8_t *reg{};
    if (opcode.family == Family::STX) {
        reg = &registers_->x;
    } else if (opcode.family == Family::STY) {
        reg = &registers_->y;
    } else if (opcode.family == Family::STA) {
        reg = &registers_->a;
    }
    result.push([=]() { mmu_->write_byte(effective_address_, *reg); });

    return result;
}

Pipeline Mos6502::create_load_instruction(Opcode opcode) {
    uint8_t *reg{};
    if (opcode.family == Family::LDX) {
        reg = &registers_->x;
    } else if (opcode.family == Family::LDY) {
        reg = &registers_->y;
    } else if (opcode.family == Family::LDA) {
        reg = &registers_->a;
    }

    Pipeline result;

    if (opcode.address_mode == AddressMode::Immediate) {
        // Empty
    } else if (opcode.address_mode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.address_mode == AddressMode::Zeropage) {
        result.append(create_zeropage_addressing_steps());
    } else if (opcode.address_mode == AddressMode::ZeropageX) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->x));
    } else if (opcode.address_mode == AddressMode::ZeropageY) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->y));
    }

    result.push([=]() {
        *reg = mmu_->read_byte(effective_address_);
        set_zero(*reg);
        set_negative(*reg);
    });

    return result;
}

Pipeline Mos6502::create_compare_instruction(Opcode opcode) {
    uint8_t *reg{};
    if (opcode.family == Family::CPX) {
        reg = &registers_->x;
    } else if (opcode.family == Family::CPY) {
        reg = &registers_->y;
    }

    Pipeline result;
    if (opcode.address_mode == AddressMode::Immediate) {
        // Empty
    } else if (opcode.address_mode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.address_mode == AddressMode::Zeropage) {
        result.append(create_zeropage_addressing_steps());
    }

    result.push([=]() {
        const uint8_t value = mmu_->read_byte(effective_address_);
        // Compare instructions are not affected be the
        // carry flag when executing the subtraction.
        const int16_t temp_result = *reg - value;
        set_carry(temp_result <= 0);
        set_zero(static_cast<uint8_t>(temp_result));
        set_negative(static_cast<uint8_t>(temp_result));
    });
    return result;
}

Pipeline Mos6502::create_zeropage_addressing_steps() {
    Pipeline result;
    result.push([=]() {
        effective_address_ = mmu_->read_byte(registers_->pc);
        ++registers_->pc;
    });
    return result;
}

Pipeline Mos6502::create_zeropage_indexed_addressing_steps(
        const uint8_t *index_reg) {
    Pipeline result;
    result.push([=]() { /* Empty */ });
    result.push([=]() {
        const uint8_t address = mmu_->read_byte(registers_->pc);
        const uint8_t effective_address_low = address + *index_reg;
        effective_address_ = effective_address_low;
        ++registers_->pc;
    });
    return result;
}

Pipeline Mos6502::create_absolute_addressing_steps() {
    Pipeline result;
    result.push([=]() { ++registers_->pc; });
    result.push([=]() {
        ++registers_->pc;
        effective_address_ = mmu_->read_word(registers_->pc - 2);
    });
    return result;
}

Pipeline Mos6502::create_absolute_indexed_addressing_steps(
        const uint8_t *index_reg) {
    Pipeline result;
    result.push([=]() { ++registers_->pc; });
    result.push([=]() {
        ++registers_->pc;
        effective_address_ = mmu_->read_word(registers_->pc - 2);
    });
    result.push([=]() {
        const uint8_t offset = *index_reg;
        effective_address_ += offset;
    });
    return result;
}

Pipeline Mos6502::create_indexed_indirect_addressing_steps() {
    Pipeline result;
    result.push([=]() { /* Empty */ });
    result.push([=]() { /* Empty */ });
    result.push([=]() { /* Empty */ });
    result.push([=]() {
        const uint8_t ptr_address = mmu_->read_byte(registers_->pc++);
        // Effective address is always fetched from zero page
        const uint8_t address = ptr_address + registers_->x;
        if (address == 0xFF) {
            // Special case where the effective address should come from
            // 0x00 and 0xFF, not 0x0100 and 0x00FF.
            const uint8_t lower = mmu_->read_byte(address);
            const uint16_t upper = mmu_->read_byte(0x00) << 8;
            effective_address_ = upper | lower;
        } else {
            effective_address_ = mmu_->read_word(address);
        }
    });
    return result;
}

Pipeline Mos6502::create_indirect_indexed_addressing_steps() {
    Pipeline result;
    result.push([=]() { /* Empty */ });
    result.push([=]() { /* Empty */ });
    result.push([=]() { /* Empty */ });
    result.push([=]() {
        const uint8_t ptr_address = mmu_->read_byte(registers_->pc++);
        const uint16_t address = mmu_->read_word(ptr_address);

        effective_address_ = address + registers_->y;
    });
    return result;
}

} // namespace n_e_s::core
