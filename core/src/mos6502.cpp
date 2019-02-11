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

// Most instruction timings are from https://robinli.eu/f/6502_cpu.txt
void Mos6502::execute() {
    if (pipeline_.empty()) {
        const uint8_t raw_opcode{mmu_->read_byte(registers_->pc++)};
        const Opcode opcode = decode(raw_opcode);

        if (opcode.addressMode == AddressMode::Immediate) {
            effective_address_ = registers_->pc++;
        }

        switch (opcode.instruction) {
        case Instruction::BRK:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                /* Do nothing. */
            });
            pipeline_.push([=]() { stack_.push_word(registers_->pc); });
            pipeline_.push([=]() { stack_.push_byte(registers_->p | B_FLAG); });
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push(
                    [=]() { registers_->pc = mmu_->read_word(kBrkAddress); });
            return;
        case Instruction::PHP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() { stack_.push_byte(registers_->p); });
            return;
        case Instruction::BPL:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & N_FLAG); }));
            return;
        case Instruction::BIT:
            if (opcode.addressMode == AddressMode::Absolute) {
                pipeline_.append(create_absolute_addressing_steps());
            } else if (opcode.addressMode == AddressMode::Zeropage) {
                pipeline_.append(create_zeropage_addressing_steps());
            } else {
                break;
            }
            pipeline_.push([=]() {
                const uint8_t value = mmu_->read_byte(effective_address_);
                set_zero(value & registers_->a);
                set_negative(value);
                if (value & (1u << 6)) {
                    set_flag(V_FLAG);
                } else {
                    clear_flag(V_FLAG);
                }
            });
            return;
        case Instruction::CLC:
            pipeline_.push([=]() { clear_flag(C_FLAG); });
            return;
        case Instruction::JSR:
            pipeline_.append(create_absolute_addressing_steps());
            pipeline_.push([=]() {
                /* Do nothing. */
            });
            pipeline_.push([=]() { stack_.push_word(--registers_->pc); });
            pipeline_.push([=]() { registers_->pc = effective_address_; });
            return;
        case Instruction::BMI:
            pipeline_.push(branch_on([=]() { return registers_->p & N_FLAG; }));
            return;
        case Instruction::SEC:
            pipeline_.push([=]() { set_flag(C_FLAG); });
            return;
        case Instruction::LSR:
            if (opcode.addressMode == AddressMode::Accumulator) {
                pipeline_.push([=]() {
                    set_carry(registers_->a & 1);
                    registers_->a &= ~1;
                    registers_->a >>= 1;
                    set_zero(registers_->a);
                    clear_flag(N_FLAG);
                });
                return;
            }
            break;
        case Instruction::PHA:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() { stack_.push_byte(registers_->a); });
            return;
        case Instruction::JMP:
            pipeline_.push([=]() { ++registers_->pc; });
            pipeline_.push([=]() {
                registers_->pc = mmu_->read_word(registers_->pc - 1);
            });
            return;
        case Instruction::BVC:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & V_FLAG); }));
            return;
        case Instruction::CLI:
            pipeline_.push([=]() { clear_flag(I_FLAG); });
            return;
        case Instruction::ADC:
            pipeline_.append(create_add_instruction(opcode));
            return;
        case Instruction::BVS:
            pipeline_.push(branch_on([=]() { return registers_->p & V_FLAG; }));
            return;
        case Instruction::SEI:
            pipeline_.push([=]() { set_flag(I_FLAG); });
            return;
        case Instruction::STA:
        case Instruction::STX:
        case Instruction::STY:
            pipeline_.append(create_store_instruction(opcode));
            return;
        case Instruction::TXS:
            pipeline_.push([=]() { registers_->sp = registers_->x; });
            return;
        case Instruction::TYA:
            pipeline_.push([=]() {
                registers_->a = registers_->y;
                set_zero(registers_->a);
                set_negative(registers_->a);
            });
            return;
        case Instruction::TAY:
            pipeline_.push([=]() {
                registers_->y = registers_->a;
                set_zero(registers_->y);
                set_negative(registers_->y);
            });
            return;
        case Instruction::TAX:
            pipeline_.push([=]() {
                registers_->x = registers_->a;
                set_zero(registers_->x);
                set_negative(registers_->x);
            });
            return;
        case Instruction::TSX:
            pipeline_.push([=]() {
                registers_->x = registers_->sp;
                set_zero(registers_->x);
                set_negative(registers_->x);
            });
            return;
        case Instruction::TXA:
            pipeline_.push([=]() {
                registers_->a = registers_->x;
                set_zero(registers_->a);
                set_negative(registers_->a);
            });
            return;
        case Instruction::BCC:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & C_FLAG); }));
            return;
        case Instruction::LDA:
        case Instruction::LDX:
        case Instruction::LDY:
            pipeline_.append(create_load_instruction(opcode));
            return;
        case Instruction::BCS:
            pipeline_.push(branch_on([=]() { return registers_->p & C_FLAG; }));
            return;
        case Instruction::CLV:
            pipeline_.push([=]() { clear_flag(V_FLAG); });
            return;
        case Instruction::BNE:
            pipeline_.push(
                    branch_on([=]() { return !(registers_->p & Z_FLAG); }));
            return;
        case Instruction::CLD:
            pipeline_.push([=]() { clear_flag(D_FLAG); });
            return;
        case Instruction::NOP:
            pipeline_.push([]() { /* Do nothing. */ });
            return;
        case Instruction::INX:
            pipeline_.push([=]() {
                ++registers_->x;
                set_zero(registers_->x);
                set_negative(registers_->x);
            });
            return;
        case Instruction::DEX:
            pipeline_.push([=]() {
                --registers_->x;
                set_zero(registers_->x);
                set_negative(registers_->x);
            });
            return;
        case Instruction::INY:
            pipeline_.push([=]() {
                ++registers_->y;
                set_zero(registers_->y);
                set_negative(registers_->y);
            });
            return;
        case Instruction::DEY:
            pipeline_.push([=]() {
                --registers_->y;
                set_zero(registers_->y);
                set_negative(registers_->y);
            });
            return;
        case Instruction::BEQ:
            pipeline_.push(branch_on([=]() { return registers_->p & Z_FLAG; }));
            return;
        case Instruction::SED:
            pipeline_.push([=]() { set_flag(D_FLAG); });
            return;
        case Instruction::Invalid:
            break;
        }
        std::stringstream err;
        err << "Bad instruction: " << std::showbase << std::hex << +raw_opcode;
        throw std::logic_error(err.str());
    }

    pipeline_.execute_step();
}

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

std::function<void()> Mos6502::branch_on(
        const std::function<bool()> &condition) {
    return [=]() {
        if (!condition()) {
            ++registers_->pc;
            return;
        }

        pipeline_.push([=]() {
            const uint8_t offset = mmu_->read_byte(registers_->pc++);
            const uint16_t page = high_byte(registers_->pc);

            registers_->pc += to_signed(offset);

            if (page != high_byte(registers_->pc)) {
                // We crossed a page boundary so we spend 1 more cycle.
                pipeline_.push([=]() { /* Do nothing. */ });
            }
        });
    };
}

Pipeline Mos6502::create_add_instruction(Opcode opcode) {
    Pipeline result;
    if (opcode.addressMode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.addressMode == AddressMode::Zeropage) {
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
    if (opcode.addressMode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.addressMode == AddressMode::Zeropage) {
        result.append(create_zeropage_addressing_steps());
    } else if (opcode.addressMode == AddressMode::ZeropageX) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->x));
    } else if (opcode.addressMode == AddressMode::ZeropageY) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->y));
    }

    uint8_t *reg{};
    if (opcode.instruction == Instruction::STX) {
        reg = &registers_->x;
    } else if (opcode.instruction == Instruction::STY) {
        reg = &registers_->y;
    } else if (opcode.instruction == Instruction::STA) {
        reg = &registers_->a;
    }
    result.push([=]() { mmu_->write_byte(effective_address_, *reg); });

    return result;
}

Pipeline Mos6502::create_load_instruction(Opcode opcode) {
    uint8_t *reg{};
    if (opcode.instruction == Instruction::LDX) {
        reg = &registers_->x;
    } else if (opcode.instruction == Instruction::LDY) {
        reg = &registers_->y;
    } else if (opcode.instruction == Instruction::LDA) {
        reg = &registers_->a;
    }

    Pipeline result;

    if (opcode.addressMode == AddressMode::Immediate) {
        // Empty
    } else if (opcode.addressMode == AddressMode::Absolute) {
        result.append(create_absolute_addressing_steps());
    } else if (opcode.addressMode == AddressMode::Zeropage) {
        result.append(create_zeropage_addressing_steps());
    } else if (opcode.addressMode == AddressMode::ZeropageX) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->x));
    } else if (opcode.addressMode == AddressMode::ZeropageY) {
        result.append(create_zeropage_indexed_addressing_steps(&registers_->y));
    }

    result.push([=]() {
        *reg = mmu_->read_byte(effective_address_);
        set_zero(*reg);
        set_negative(*reg);
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

} // namespace n_e_s::core
