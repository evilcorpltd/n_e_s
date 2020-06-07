#include "mos6502.h"

#include "nes/core/opcode.h"

#include <sstream>
#include <stdexcept>

namespace {

const uint16_t kResetAddress = 0xFFFC; // This is where the reset routine is.
const uint16_t kBrkAddress = 0xFFFE; // This is where the break routine is.

constexpr bool is_negative(uint8_t byte) {
    return (byte & (1u << 7u)) != 0;
}

constexpr uint8_t low_bits(uint8_t byte) {
    return static_cast<uint8_t>(
            byte & static_cast<uint8_t>(~static_cast<uint8_t>(1u << 7u)));
}

constexpr int8_t to_signed(uint8_t byte) {
    if (is_negative(byte)) {
        return low_bits(byte) - static_cast<uint8_t>(128);
    }

    return low_bits(byte);
}

constexpr uint16_t high_byte(uint16_t word) {
    return word & static_cast<uint16_t>(0xFF00u);
}

// Returns true if accessing an index address will require the cpu to reach
// across a page boundary.
constexpr bool cross_page(uint16_t address, uint8_t index) {
    return (static_cast<uint16_t>(address + index) & 0xFF00u) !=
           (address & 0xFF00u);
}

} // namespace

namespace n_e_s::core {

Mos6502::Stack::Stack(CpuRegisters *registers, IMmu *mmu)
        : registers_(registers), mmu_(mmu) {}

uint8_t Mos6502::Stack::pop_byte() {
    return mmu_->read_byte(ram_offset_ + ++registers_->sp);
}

void Mos6502::Stack::push_byte(uint8_t byte) {
    mmu_->write_byte(ram_offset_ + registers_->sp--, byte);
}

Mos6502::Mos6502(CpuRegisters *const registers, IMmu *const mmu)
        : registers_(registers),
          mmu_(mmu),
          stack_(registers_, mmu_),
          pipeline_() {}

void Mos6502::execute() {
    if (pipeline_.done()) {
        if (nmi_) {
            pipeline_ = create_nmi();
            nmi_ = false;
        } else {
            pipeline_ = parse_next_instruction();
        }
    } else {
        pipeline_.execute_step();
    }
    ++state_.cycle;
}

// Most instruction timings are from https://robinli.eu/f/6502_cpu.txt
Pipeline Mos6502::parse_next_instruction() {
    state_.start_pc = registers_->pc;
    state_.start_cycle = state_.cycle;

    const uint8_t raw_opcode{mmu_->read_byte(registers_->pc++)};
    state_.current_opcode = decode(raw_opcode);

    const MemoryAccess memory_access =
            get_memory_access(state_.current_opcode->family);

    if (state_.current_opcode->family == Family::Invalid) {
        std::stringstream err;
        err << "Bad instruction: " << std::showbase << std::hex << +raw_opcode;
        err << " @ " << registers_->pc - 1;
        throw std::logic_error(err.str());
    }

    if (state_.current_opcode->address_mode == AddressMode::Immediate) {
        effective_address_ = registers_->pc++;
    }

    Pipeline result;
    switch (state_.current_opcode->instruction) {
    case Instruction::BrkImplied:
        result.push([this]() {
            // Dummy read
            mmu_->read_byte(registers_->pc++);
        });
        result.push([this]() {
            stack_.push_byte(static_cast<uint8_t>(registers_->pc >> 8u));
        });
        result.push([this]() {
            stack_.push_byte(static_cast<uint8_t>(registers_->pc & 0xFFu));
        });
        result.push([this]() { stack_.push_byte(registers_->p | B_FLAG); });
        result.push([this]() { tmp_ = mmu_->read_byte(kBrkAddress); });
        result.push([this]() {
            const uint16_t pch = mmu_->read_byte(kBrkAddress + 1) << 8u;
            registers_->pc = pch | tmp_;
        });
        break;
    case Instruction::AslZeropage:
    case Instruction::AslAccumulator:
    case Instruction::AslAbsolute:
    case Instruction::AslZeropageX:
    case Instruction::AslAbsoluteX:
        result.append(
                create_left_shift_instruction(*state_.current_opcode, false));
        break;
    case Instruction::PhpImplied:
        result.push([]() { /* Do nothing. */ });
        result.push([this]() { stack_.push_byte(registers_->p | B_FLAG); });
        break;
    case Instruction::BplRelative:
        result.append(create_branch_instruction(
                [this]() { return !(registers_->p & N_FLAG); }));
        break;
    case Instruction::BitZeropage:
    case Instruction::BitAbsolute:
        result.append(create_addressing_steps(
                state_.current_opcode->address_mode, memory_access));

        result.push([this]() {
            const uint8_t value = mmu_->read_byte(effective_address_);
            set_zero(value & registers_->a);
            set_negative(value);
            if (value & (1u << 6u)) {
                set_flag(V_FLAG);
            } else {
                clear_flag(V_FLAG);
            }
        });
        break;
    case Instruction::PlpImplied:
        result.push([]() { /* Do nothing. */ });
        result.push([]() { /* Do nothing. */ });
        result.push([this]() {
            registers_->p = stack_.pop_byte();
            set_flag(FLAG_5);
            clear_flag(B_FLAG);
        });
        break;
    case Instruction::AndIndirectX:
    case Instruction::AndImmediate:
    case Instruction::AndAbsolute:
    case Instruction::AndAbsoluteX:
    case Instruction::AndAbsoluteY:
    case Instruction::AndZeropage:
    case Instruction::AndIndirectY:
    case Instruction::AndZeropageX:
        result.append(create_and_instruction(*state_.current_opcode));
        break;
    case Instruction::ClcImplied:
        result.push([this]() { clear_flag(C_FLAG); });
        break;
    case Instruction::JsrAbsolute:
        result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
        result.push([]() { /* Do nothing. */ });
        result.push([this]() {
            const auto pch = static_cast<uint8_t>(registers_->pc >> 8u);
            stack_.push_byte(pch);
        });
        result.push([this]() {
            const auto pcl = static_cast<uint8_t>(registers_->pc);
            stack_.push_byte(pcl);
        });
        result.push([this]() {
            const uint16_t pch = mmu_->read_byte(registers_->pc) << 8u;
            registers_->pc = pch | tmp_;
        });
        break;
    case Instruction::BmiRelative:
        result.append(create_branch_instruction(
                [this]() { return registers_->p & N_FLAG; }));
        break;
    case Instruction::SecImplied:
        result.push([this]() { set_flag(C_FLAG); });
        break;
    case Instruction::LsrZeropage:
    case Instruction::LsrAbsolute:
    case Instruction::LsrAccumulator:
    case Instruction::LsrAbsoluteX:
    case Instruction::LsrZeropageX:
        result.append(
                create_right_shift_instruction(*state_.current_opcode, false));
        break;
    case Instruction::PhaImplied:
        result.push([]() { /* Do nothing. */ });
        result.push([this]() { stack_.push_byte(registers_->a); });
        break;
    case Instruction::JmpAbsolute:
        result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
        result.push([this]() {
            const uint16_t pch = mmu_->read_byte(registers_->pc) << 8u;
            registers_->pc = pch | tmp_;
        });
        break;
    case Instruction::JmpIndirect:
        result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
        result.push([this]() { tmp2_ = mmu_->read_byte(registers_->pc++); });
        result.push([this]() {
            const uint16_t ptraddress =
                    static_cast<uint16_t>(tmp2_ << 8u) | tmp_;
            effective_address_ = mmu_->read_byte(ptraddress);
        });
        result.push([this]() {
            // The PCH will always be fetched from the same page
            // as PCL, i.e. page boundary crossing is not handled.
            const uint8_t low_address = tmp_ + 1u;
            const uint16_t ptraddress =
                    static_cast<uint16_t>(tmp2_ << 8u) | low_address;
            const uint16_t pch = mmu_->read_byte(ptraddress) << 8u;
            registers_->pc = effective_address_ | pch;
        });
        break;
    case Instruction::BvcRelative:
        result.append(create_branch_instruction(
                [this]() { return !(registers_->p & V_FLAG); }));
        break;
    case Instruction::CliImplied:
        result.push([this]() { clear_flag(I_FLAG); });
        break;
    case Instruction::AdcZeropage:
    case Instruction::AdcZeropageX:
    case Instruction::AdcImmediate:
    case Instruction::AdcAbsolute:
    case Instruction::AdcAbsoluteX:
    case Instruction::AdcAbsoluteY:
    case Instruction::AdcIndirectX:
    case Instruction::AdcIndirectY:
        result.append(create_add_instruction(*state_.current_opcode));
        break;
    case Instruction::SbcZeropage:
    case Instruction::SbcZeropageX:
    case Instruction::SbcImmediate:
    case Instruction::SbcImmediateEB:
    case Instruction::SbcAbsolute:
    case Instruction::SbcAbsoluteX:
    case Instruction::SbcAbsoluteY:
    case Instruction::SbcIndirectX:
    case Instruction::SbcIndirectY:
        result.append(create_sub_instruction(*state_.current_opcode));
        break;
    case Instruction::IsbZeropage:
    case Instruction::IsbZeropageX:
    case Instruction::IsbAbsolute:
    case Instruction::IsbAbsoluteX:
    case Instruction::IsbAbsoluteY:
    case Instruction::IsbIndirectIndexed:
    case Instruction::IsbIndexedIndirect:
        result.append(create_isb_instruction(*state_.current_opcode));
        break;
    case Instruction::PlaImplied:
        result.push([]() { /* Do nothing. */ });
        result.push([]() { /* Do nothing. */ });
        result.push([this]() {
            registers_->a = stack_.pop_byte();
            set_zero(registers_->a);
            set_negative(registers_->a);
        });
        break;
    case Instruction::RtsImplied:
        result.push([this]() {
            // Dummy read
            mmu_->read_byte(registers_->pc);
        });
        result.push([]() { /* Do nothing. */ });
        result.push([this]() { tmp_ = stack_.pop_byte(); });
        result.push([this]() {
            const uint16_t pch = stack_.pop_byte() << 8u;
            registers_->pc = pch | tmp_;
        });
        result.push([this]() { ++registers_->pc; });
        break;
    case Instruction::RtiImplied:
        result.push([this]() {
            // Dummy read
            mmu_->read_byte(registers_->pc);
        });
        result.push([]() { /* Do nothing. */ });
        result.push([this]() {
            registers_->p = stack_.pop_byte();
            set_flag(FLAG_5);
            clear_flag(B_FLAG);
        });
        result.push([this]() { tmp_ = stack_.pop_byte(); });
        result.push([this]() {
            const uint16_t pch = stack_.pop_byte() << 8u;
            registers_->pc = pch | tmp_;
        });
        break;
    case Instruction::BvsRelative:
        result.append(create_branch_instruction(
                [this]() { return registers_->p & V_FLAG; }));
        break;
    case Instruction::SeiImplied:
        result.push([this]() { set_flag(I_FLAG); });
        break;
    case Instruction::StaIndexedIndirect:
    case Instruction::StaZeropage:
    case Instruction::StaAbsolute:
    case Instruction::StaIndirectIndexed:
    case Instruction::StaZeropageX:
    case Instruction::StaAbsoluteY:
    case Instruction::StaAbsoluteX:
    case Instruction::StxZeropage:
    case Instruction::StxAbsolute:
    case Instruction::StxZeropageY:
    case Instruction::StyZeropage:
    case Instruction::StyAbsolute:
    case Instruction::StyZeropageX:
    case Instruction::SaxAbsolute:
    case Instruction::SaxZeropage:
    case Instruction::SaxZeropageY:
    case Instruction::SaxIndirectX:
        result.append(create_store_instruction(*state_.current_opcode));
        break;
    case Instruction::TxsImplied:
        result.push([this]() { registers_->sp = registers_->x; });
        break;
    case Instruction::TyaImplied:
        result.push([this]() {
            registers_->a = registers_->y;
            set_zero(registers_->a);
            set_negative(registers_->a);
        });
        break;
    case Instruction::TayImplied:
        result.push([this]() {
            registers_->y = registers_->a;
            set_zero(registers_->y);
            set_negative(registers_->y);
        });
        break;
    case Instruction::TaxImplied:
        result.push([this]() {
            registers_->x = registers_->a;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::TsxImplied:
        result.push([this]() {
            registers_->x = registers_->sp;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::TxaImplied:
        result.push([this]() {
            registers_->a = registers_->x;
            set_zero(registers_->a);
            set_negative(registers_->a);
        });
        break;
    case Instruction::BccRelative:
        result.append(create_branch_instruction(
                [this]() { return !(registers_->p & C_FLAG); }));
        break;
    case Instruction::LdaZeropage:
    case Instruction::LdaImmediate:
    case Instruction::LdaAbsolute:
    case Instruction::LdaZeropageX:
    case Instruction::LdaIndirectX:
    case Instruction::LdaIndirectY:
    case Instruction::LdxImmediate:
    case Instruction::LdxZeropage:
    case Instruction::LdxAbsolute:
    case Instruction::LdxZeropageY:
    case Instruction::LdyImmediate:
    case Instruction::LdyZeropage:
    case Instruction::LdyAbsolute:
    case Instruction::LdyZeropageX:
    case Instruction::LdaAbsoluteY:
    case Instruction::LdyAbsoluteX:
    case Instruction::LdaAbsoluteX:
    case Instruction::LdxAbsoluteY:
        result.append(create_load_instruction(*state_.current_opcode));
        break;
    case Instruction::BcsRelative:
        result.append(create_branch_instruction(
                [this]() { return registers_->p & C_FLAG; }));
        break;
    case Instruction::ClvImplied:
        result.push([this]() { clear_flag(V_FLAG); });
        break;
    case Instruction::BneRelative:
        result.append(create_branch_instruction(
                [this]() { return !(registers_->p & Z_FLAG); }));
        break;
    case Instruction::CldImplied:
        result.push([this]() { clear_flag(D_FLAG); });
        break;
    case Instruction::CpxImmediate:
    case Instruction::CpxZeropage:
    case Instruction::CpxAbsolute:
    case Instruction::CpyImmediate:
    case Instruction::CpyZeropage:
    case Instruction::CpyAbsolute:
    case Instruction::CmpImmediate:
    case Instruction::CmpZeropage:
    case Instruction::CmpZeropageX:
    case Instruction::CmpAbsolute:
    case Instruction::CmpAbsoluteX:
    case Instruction::CmpAbsoluteY:
    case Instruction::CmpIndirectX:
    case Instruction::CmpIndirectY:
        result.append(create_compare_instruction(*state_.current_opcode));
        break;
    case Instruction::NopImplied:
    case Instruction::NopImplied1A:
    case Instruction::NopImplied3A:
    case Instruction::NopImplied5A:
    case Instruction::NopImplied7A:
    case Instruction::NopImpliedDA:
    case Instruction::NopImpliedFA:
    case Instruction::NopImmediate80:
    case Instruction::NopZeropage04:
    case Instruction::NopZeropageX14:
    case Instruction::NopZeropageX34:
    case Instruction::NopZeropageX54:
    case Instruction::NopZeropageX74:
    case Instruction::NopZeropageXD4:
    case Instruction::NopZeropageXF4:
    case Instruction::NopZeropage44:
    case Instruction::NopZeropage64:
    case Instruction::NopAbsolute0C:
    case Instruction::NopAbsoluteX1C:
    case Instruction::NopAbsoluteX3C:
    case Instruction::NopAbsoluteX5C:
    case Instruction::NopAbsoluteX7C:
    case Instruction::NopAbsoluteXDC:
    case Instruction::NopAbsoluteXFC:
        result.append(create_addressing_steps(
                state_.current_opcode->address_mode, memory_access));
        result.push([]() { /* Do nothing. */ });
        break;
    case Instruction::IncZeropage:
    case Instruction::IncZeropageX:
    case Instruction::IncAbsoluteX:
    case Instruction::IncAbsolute:
        result.append(create_inc_instruction(*state_.current_opcode));
        break;
    case Instruction::DecZeropage:
    case Instruction::DecZeropageX:
    case Instruction::DecAbsoluteX:
    case Instruction::DecAbsolute:
        result.append(create_dec_instruction(*state_.current_opcode));
        break;
    case Instruction::InxImplied:
        result.push([this]() {
            ++registers_->x;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::DexImplied:
        result.push([this]() {
            --registers_->x;
            set_zero(registers_->x);
            set_negative(registers_->x);
        });
        break;
    case Instruction::InyImplied:
        result.push([this]() {
            ++registers_->y;
            set_zero(registers_->y);
            set_negative(registers_->y);
        });
        break;
    case Instruction::DeyImplied:
        result.push([this]() {
            --registers_->y;
            set_zero(registers_->y);
            set_negative(registers_->y);
        });
        break;
    case Instruction::BeqRelative:
        result.append(create_branch_instruction(
                [this]() { return registers_->p & Z_FLAG; }));
        break;
    case Instruction::SedImplied:
        result.push([this]() { set_flag(D_FLAG); });
        break;
    case Instruction::EorIndirectX:
    case Instruction::EorZeropage:
    case Instruction::EorZeropageX:
    case Instruction::EorImmediate:
    case Instruction::EorIndirectY:
    case Instruction::EorAbsolute:
    case Instruction::EorAbsoluteX:
    case Instruction::EorAbsoluteY:
        result.append(create_eor_instruction(*state_.current_opcode));
        break;
    case Instruction::RolAccumulator:
    case Instruction::RolZeropage:
    case Instruction::RolAbsolute:
    case Instruction::RolZeropageX:
    case Instruction::RolAbsoluteX:
        result.append(
                create_left_shift_instruction(*state_.current_opcode, true));
        break;
    case Instruction::RorZeropage:
    case Instruction::RorAbsolute:
    case Instruction::RorAccumulator:
    case Instruction::RorZeropageX:
    case Instruction::RorAbsoluteX:
        result.append(
                create_right_shift_instruction(*state_.current_opcode, true));
        break;
    case Instruction::OraIndexedIndirect:
    case Instruction::OraIndirectIndexed:
    case Instruction::OraImmediate:
    case Instruction::OraAbsolute:
    case Instruction::OraAbsoluteY:
    case Instruction::OraAbsoluteX:
    case Instruction::OraZeropage:
    case Instruction::OraZeropageX:
        result.append(create_ora_instruction(*state_.current_opcode));
        break;
    case Instruction::LaxZeropage:
    case Instruction::LaxZeropageY:
    case Instruction::LaxAbsolute:
    case Instruction::LaxAbsoluteY:
    case Instruction::LaxIndirectX:
    case Instruction::LaxIndirectY:
        result.append(create_load_instruction(*state_.current_opcode));
        break;
    case Instruction::DcpZeropage:
    case Instruction::DcpZeropageX:
    case Instruction::DcpAbsolute:
    case Instruction::DcpAbsoluteX:
    case Instruction::DcpAbsoluteY:
    case Instruction::DcpIndirectIndexed:
    case Instruction::DcpIndexedIndirect:
        result.append(create_dcp_instruction(*state_.current_opcode));
        break;
    }
    return result;
} // namespace n_e_s::core

void Mos6502::reset() {
    pipeline_.clear();
    nmi_ = false;

    const uint16_t lower = mmu_->read_byte(kResetAddress);
    const uint16_t upper = mmu_->read_byte(kResetAddress + 1u) << 8u;
    registers_->pc = upper | lower;
}

const CpuState &Mos6502::state() const {
    return state_;
}

void Mos6502::set_nmi(bool nmi) {
    nmi_ = nmi;
}

void Mos6502::clear_flag(uint8_t flag) {
    registers_->p &= static_cast<uint8_t>(~flag);
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
    auto a = static_cast<uint8_t>(reg_value ^ resulting_value);
    auto b = static_cast<uint8_t>(operand ^ resulting_value);
    const bool overflow = (static_cast<uint8_t>(a & b) & 0x80u) != 0;
    if (overflow) {
        set_flag(V_FLAG);
    } else {
        clear_flag(V_FLAG);
    }
}

Pipeline Mos6502::create_nmi() {
    // Dummy read
    mmu_->read_byte(registers_->pc);
    Pipeline result;

    result.push([this]() {
        // Dummy read
        mmu_->read_byte(registers_->pc);
    });
    result.push([this]() {
        stack_.push_byte(static_cast<uint8_t>(registers_->pc >> 8u));
    });
    result.push([this]() {
        stack_.push_byte(static_cast<uint8_t>(registers_->pc & 0xFFu));
    });
    result.push([this]() { stack_.push_byte(registers_->p); });
    result.push([this]() { tmp_ = mmu_->read_byte(0xFFFA); });
    result.push([this]() {
        const uint16_t pch = mmu_->read_byte(0xFFFB) << 8u;
        registers_->pc = pch | tmp_;
    });

    return result;
}

Pipeline Mos6502::create_branch_instruction(
        const std::function<bool()> &condition) {
    Pipeline result;

    result.push_conditional([this, condition]() {
        if (!condition()) {
            ++registers_->pc;
            return StepResult::Stop;
        }
        return StepResult::Continue;
    });

    result.push_conditional([this]() {
        const uint8_t offset = mmu_->read_byte(registers_->pc++);
        const uint16_t page = high_byte(registers_->pc);

        registers_->pc += to_signed(offset);

        if (page != high_byte(registers_->pc)) {
            // We crossed a page boundary so we spend 1 more cycle.
            return StepResult::Continue;
        }
        return StepResult::Stop;
    });

    result.push([]() { /* Do nothing. */ });

    return result;
}

Pipeline Mos6502::create_inc_instruction(const Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        const uint8_t new_value = tmp_ + static_cast<uint8_t>(1);
        set_zero(new_value);
        set_negative(new_value);
        mmu_->write_byte(effective_address_, new_value);
    });

    return result;
}

Pipeline Mos6502::create_dec_instruction(const Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        const uint8_t new_value = tmp_ - static_cast<uint8_t>(1);
        set_zero(new_value);
        set_negative(new_value);
        mmu_->write_byte(effective_address_, new_value);
    });

    return result;
}

void Mos6502::adc_impl(const uint8_t addend) {
    const uint8_t a_before = registers_->a;
    const uint8_t carry = registers_->p & C_FLAG ? static_cast<uint8_t>(1u)
                                                 : static_cast<uint8_t>(0u);
    const uint16_t temp_result = registers_->a + addend + carry;
    registers_->a = static_cast<uint8_t>(temp_result);

    set_carry(temp_result > 0xFF);
    set_zero(registers_->a);
    set_negative(registers_->a);
    set_overflow(a_before, addend, temp_result);
}

Pipeline Mos6502::create_add_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        const uint8_t addend = mmu_->read_byte(effective_address_);
        adc_impl(addend);
    });

    return result;
}

Pipeline Mos6502::create_sub_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        // SBC simply takes the ones complement of the second value and then
        // performs an ADC See:
        // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
        const uint8_t addend = mmu_->read_byte(effective_address_);
        adc_impl(~addend);
    });

    return result;
}

Pipeline Mos6502::create_isb_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    // ISB = INC + SBC
    result.push([this]() {
        const uint8_t new_value = tmp_ + static_cast<uint8_t>(1);
        set_zero(new_value);
        set_negative(new_value);
        mmu_->write_byte(effective_address_, new_value);

        // SBC simply takes the ones complement of the second value and then
        // performs an ADC See:
        // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
        const uint8_t addend = new_value;
        adc_impl(~addend);
    });

    return result;
}

Pipeline Mos6502::create_and_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        const uint8_t operand = mmu_->read_byte(effective_address_);
        registers_->a &= operand;

        set_zero(registers_->a);
        set_negative(registers_->a);
    });

    return result;
}
Pipeline Mos6502::create_store_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    uint8_t *reg{};
    uint8_t *reg2{};
    if (opcode.family == Family::STX) {
        reg = &registers_->x;
    } else if (opcode.family == Family::STY) {
        reg = &registers_->y;
    } else if (opcode.family == Family::STA) {
        reg = &registers_->a;
    } else if (opcode.family == Family::SAX) {
        reg = &registers_->a;
        reg2 = &registers_->x;
    }
    result.push([this, reg, reg2]() {
        const uint8_t value = reg2 == nullptr ? *reg : *reg & *reg2;
        mmu_->write_byte(effective_address_, value);
    });

    return result;
}

Pipeline Mos6502::create_load_instruction(Opcode opcode) {
    uint8_t *reg{};
    uint8_t *reg2{};
    if (opcode.family == Family::LDX) {
        reg = &registers_->x;
    } else if (opcode.family == Family::LDY) {
        reg = &registers_->y;
    } else if (opcode.family == Family::LDA) {
        reg = &registers_->a;
    } else if (opcode.family == Family::LAX) {
        reg = &registers_->a;
        reg2 = &registers_->x;
    }

    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this, reg, reg2]() {
        const uint8_t value = mmu_->read_byte(effective_address_);
        *reg = value;
        set_zero(value);
        set_negative(value);
        if (reg2) {
            *reg2 = value;
        }
    });

    return result;
}

Pipeline Mos6502::create_compare_instruction(Opcode opcode) {
    uint8_t *reg{};
    if (opcode.family == Family::CPX) {
        reg = &registers_->x;
    } else if (opcode.family == Family::CPY) {
        reg = &registers_->y;
    } else if (opcode.family == Family::CMP) {
        reg = &registers_->a;
    }

    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));
    result.push([this, reg]() {
        const uint8_t value = mmu_->read_byte(effective_address_);
        // Compare instructions are not affected be the
        // carry flag when executing the subtraction.
        const int16_t temp_result = *reg - value;
        set_carry(*reg >= value);
        set_zero(static_cast<uint8_t>(temp_result));
        set_negative(static_cast<uint8_t>(temp_result));
    });
    return result;
}

Pipeline Mos6502::create_dcp_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));
    result.push([this]() {
        // DEC
        const uint8_t new_value = tmp_ - static_cast<uint8_t>(1);
        mmu_->write_byte(effective_address_, new_value);

        // CMP
        const uint8_t reg = registers_->a;

        set_carry(reg >= new_value);
        const int16_t temp_result = reg - new_value;
        set_zero(static_cast<uint8_t>(temp_result));
        set_negative(static_cast<uint8_t>(temp_result));
    });
    return result;
}

Pipeline Mos6502::create_eor_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        const uint8_t operand = mmu_->read_byte(effective_address_);
        registers_->a ^= operand;

        set_zero(registers_->a);
        set_negative(registers_->a);
    });

    return result;
}

Pipeline Mos6502::create_ora_instruction(Opcode opcode) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this]() {
        const uint8_t operand = mmu_->read_byte(effective_address_);
        registers_->a |= operand;

        set_zero(registers_->a);
        set_negative(registers_->a);
    });

    return result;
}

Pipeline Mos6502::create_left_shift_instruction(Opcode opcode,
        bool shift_in_carry) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this, opcode, shift_in_carry]() {
        const uint8_t source_value =
                opcode.address_mode == AddressMode::Accumulator ? registers_->a
                                                                : tmp_;

        uint16_t temp_result = source_value << 1u;
        if (shift_in_carry) {
            const uint8_t carry = registers_->p & C_FLAG ? 0x01u : 0x00u;
            temp_result |= carry;
        }
        const auto result_8bit = static_cast<uint8_t>(temp_result);
        set_carry(temp_result > 0xFF);
        set_zero(result_8bit);
        set_negative(result_8bit);

        if (opcode.address_mode == AddressMode::Accumulator) {
            registers_->a = result_8bit;
        } else {
            mmu_->write_byte(effective_address_, result_8bit);
        }
    });
    return result;
}

Pipeline Mos6502::create_right_shift_instruction(Opcode opcode,
        bool shift_in_carry) {
    const MemoryAccess memory_access = get_memory_access(opcode.family);
    Pipeline result;
    result.append(create_addressing_steps(opcode.address_mode, memory_access));

    result.push([this, opcode, shift_in_carry]() {
        const uint8_t source_value =
                opcode.address_mode == AddressMode::Accumulator ? registers_->a
                                                                : tmp_;

        uint8_t shifted_value = source_value >> 1u;
        if (shift_in_carry) {
            const auto carry = registers_->p & C_FLAG
                                       ? static_cast<uint8_t>(0b1000'0000)
                                       : static_cast<uint8_t>(0x00);
            shifted_value |= carry;
        }
        set_carry(source_value & 0x01u);
        set_zero(shifted_value);
        set_negative(shifted_value);

        if (opcode.address_mode == AddressMode::Accumulator) {
            registers_->a = shifted_value;
        } else {
            mmu_->write_byte(effective_address_, shifted_value);
        }
    });
    return result;
}

Pipeline Mos6502::create_addressing_steps(AddressMode address_mode,
        const MemoryAccess access) {
    Pipeline result;

    switch (address_mode) {
    case AddressMode::Zeropage:
        result.append(create_zeropage_addressing_steps(access));
        break;
    case AddressMode::ZeropageX:
        result.append(create_zeropage_indexed_addressing_steps(
                &registers_->x, access));
        break;
    case AddressMode::ZeropageY:
        result.append(create_zeropage_indexed_addressing_steps(
                &registers_->y, access));
        break;
    case AddressMode::Absolute:
        result.append(create_absolute_addressing_steps(access));
        break;
    case AddressMode::AbsoluteX:
        result.append(create_absolute_indexed_addressing_steps(
                &registers_->x, access));
        break;
    case AddressMode::AbsoluteY:
        result.append(create_absolute_indexed_addressing_steps(
                &registers_->y, access));
        break;
    case AddressMode::IndexedIndirect:
        result.append(create_indexed_indirect_addressing_steps(access));
        break;
    case AddressMode::IndirectIndexed:
        result.append(create_indirect_indexed_addressing_steps(access));
        break;
    default:
        break;
    }

    return result;
}

Pipeline Mos6502::create_zeropage_addressing_steps(MemoryAccess access) {
    Pipeline result;
    if (access == MemoryAccess::Read || access == MemoryAccess::Write) {
        result.push([this]() {
            effective_address_ = mmu_->read_byte(registers_->pc);
            ++registers_->pc;
        });
    } else {
        result.push([this]() {
            effective_address_ = mmu_->read_byte(registers_->pc);
            ++registers_->pc;
        });
        result.push([this]() { tmp_ = mmu_->read_byte(effective_address_); });
        result.push([this]() {
            // Extra write with the old value
            mmu_->write_byte(effective_address_, tmp_);
        });
    }
    return result;
}

Pipeline Mos6502::create_zeropage_indexed_addressing_steps(
        const uint8_t *index_reg,
        MemoryAccess access) {
    Pipeline result;
    if (access == MemoryAccess::Read || access == MemoryAccess::Write) {
        result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
        result.push([this, index_reg]() {
            // Dummy read
            mmu_->read_byte(tmp_);
            const uint8_t effective_address_low = tmp_ + *index_reg;
            effective_address_ = effective_address_low;
        });
    } else if (access == MemoryAccess::ReadWrite) {
        result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
        result.push([this, index_reg]() {
            // Dummy read
            mmu_->read_byte(tmp_);
            const uint8_t effective_address_low = tmp_ + *index_reg;
            effective_address_ = effective_address_low;
        });
        result.push([this]() { tmp_ = mmu_->read_byte(effective_address_); });
        result.push([this]() {
            // Extra write to effective address with old value.
            mmu_->write_byte(effective_address_, tmp_);
        });
    }
    return result;
}

Pipeline Mos6502::create_absolute_addressing_steps(const MemoryAccess access) {
    Pipeline result;
    result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
    result.push([this]() {
        const uint16_t upper = mmu_->read_byte(registers_->pc++) << 8u;
        effective_address_ = upper | tmp_;
    });
    if (access == MemoryAccess::ReadWrite) {
        result.push([this]() { tmp_ = mmu_->read_byte(effective_address_); });
        result.push([this]() {
            // Extra write with the old value
            mmu_->write_byte(effective_address_, tmp_);
        });
    }
    return result;
}

Pipeline Mos6502::create_absolute_indexed_addressing_steps(
        const uint8_t *index_reg,
        const MemoryAccess access) {
    Pipeline result;
    result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
    result.push([this, index_reg]() {
        const uint16_t address_high = mmu_->read_byte(registers_->pc++) << 8u;
        const uint16_t abs_address = address_high | tmp_;
        const uint8_t offset = *index_reg;

        is_crossing_page_boundary_ = cross_page(abs_address, offset);
        effective_address_ = abs_address + offset;
    });

    if (access == MemoryAccess::Write) {
        result.push([this]() {
            if (is_crossing_page_boundary_) {
                // The high byte of the effective address is invalid
                // at this time (smaller by $100), but a read is still
                // performed.
                mmu_->read_byte(
                        effective_address_ - static_cast<uint16_t>(0x0100));
            } else {
                // Extra read from effective address.
                mmu_->read_byte(effective_address_);
            }
        });
    } else if (access == MemoryAccess::Read) {
        result.push_conditional([this]() {
            if (is_crossing_page_boundary_) {
                // The high byte of the effective address is invalid
                // at this time (smaller by $100), but a read is still
                // performed.
                mmu_->read_byte(
                        effective_address_ - static_cast<uint16_t>(0x0100));
                return StepResult::Continue;
            }
            return StepResult::Skip;
        });
    } else {
        result.push([this]() {
            if (is_crossing_page_boundary_) {
                // The high byte of the effective address is invalid
                // at this time (smaller by $100), but a read is still
                // performed.
                mmu_->read_byte(
                        effective_address_ - static_cast<uint16_t>(0x0100));
            } else {
                // Extra read from effective address.
                mmu_->read_byte(effective_address_);
            }
        });
        result.push([this]() {
            // Extra read from effective address.
            tmp_ = mmu_->read_byte(effective_address_);
        });
        result.push([this]() {
            // Extra write.
            mmu_->write_byte(effective_address_, tmp_);
        });
    }
    return result;
}

Pipeline Mos6502::create_indexed_indirect_addressing_steps(
        const MemoryAccess access) {
    Pipeline result;
    result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
    result.push([this]() {
        // Dummy read
        mmu_->read_byte(tmp_);
    });
    result.push([this]() {
        const uint8_t address = tmp_ + registers_->x;
        tmp2_ = mmu_->read_byte(address);
    });
    result.push([this]() {
        // Effective address is always fetched from zero page
        const uint8_t address = tmp_ + registers_->x + 1u;
        const uint16_t upper = mmu_->read_byte(address) << 8u;
        effective_address_ = upper | tmp2_;
    });
    if (access == MemoryAccess::ReadWrite) {
        result.push([this]() { tmp_ = mmu_->read_byte(effective_address_); });
        result.push([this]() { mmu_->write_byte(effective_address_, tmp_); });
    }
    return result;
}

Pipeline Mos6502::create_indirect_indexed_addressing_steps(
        const MemoryAccess access) {
    Pipeline result;
    result.push([this]() { tmp_ = mmu_->read_byte(registers_->pc++); });
    result.push([this]() { tmp2_ = mmu_->read_byte(tmp_); });
    result.push([this]() {
        // The effective address is always fetched from zero page
        const uint16_t upper = mmu_->read_byte(static_cast<uint8_t>(tmp_ + 1u))
                               << 8u;
        const uint16_t address = upper | tmp2_;
        const uint8_t offset = registers_->y;

        is_crossing_page_boundary_ = cross_page(address, offset);
        effective_address_ = address + offset;
    });
    if (access == MemoryAccess::Write) {
        result.push([this]() {
            if (is_crossing_page_boundary_) {
                // The high byte of the effective address is invalid
                // at this time (smaller by $100), but a read is still
                // performed.
                mmu_->read_byte(
                        effective_address_ - static_cast<uint16_t>(0x0100));
            } else {
                // Extra read from effective address.
                mmu_->read_byte(effective_address_);
            }
        });
    } else if (access == MemoryAccess::Read) {
        result.push_conditional([this]() {
            if (is_crossing_page_boundary_) {
                // The high byte of the effective address is invalid
                // at this time (smaller by $100), but a read is still
                // performed.
                mmu_->read_byte(
                        effective_address_ - static_cast<uint16_t>(0x0100));
                return StepResult::Continue;
            }
            return StepResult::Skip;
        });
    } else {
        result.push([this]() {
            if (is_crossing_page_boundary_) {
                // The high byte of the effective address is invalid
                // at this time (smaller by $100), but a read is still
                // performed.
                mmu_->read_byte(
                        effective_address_ - static_cast<uint16_t>(0x0100));
            } else {
                // Extra read from effective address.
                mmu_->read_byte(effective_address_);
            }
        });
        result.push([this]() { tmp_ = mmu_->read_byte(effective_address_); });
        result.push([this]() { mmu_->write_byte(effective_address_, tmp_); });
    }
    return result;
}

} // namespace n_e_s::core
