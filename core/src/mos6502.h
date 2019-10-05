#pragma once

#include "core/icpu.h"
#include "core/immu.h"
#include "core/opcode.h"
#include "pipeline.h"

#include <cstdint>

namespace n_e_s::core {

class Mos6502 : public ICpu {
public:
    // Assumes ownership of nothing. Really irresponsible.
    Mos6502(CpuRegisters *registers, IMmu *mmu);

    // ICpu
    void execute() override;
    void reset() override;

private:
    CpuRegisters *const registers_;
    IMmu *const mmu_;

    // Wraps the mmu to provide more convenient access to the stack.
    // All functions have side effects on the stack pointer, so it'll always
    // point to the next available address in the mmu's stack area.
    class Stack {
    public:
        Stack(CpuRegisters *registers, IMmu *mmu);

        uint8_t pop_byte();
        uint16_t pop_word();

        void push_byte(uint8_t byte);
        void push_word(uint16_t word);

    private:
        CpuRegisters *const registers_;
        IMmu *const mmu_;
        const uint16_t ram_offset_{0x0100};
    };

    Stack stack_;

    // Holds the atoms staged to be executed.
    Pipeline pipeline_;

    // Effective address calculated by an address mode during pipeline
    // execution.
    uint16_t effective_address_{};

    // Holds data for the CPU between pipeline steps.
    uint8_t tmp_{};

    // Set to true if accessing a memory location will require a page boundary
    // crossing.
    bool is_crossing_page_boundary_{false};

    void clear_flag(uint8_t flag);
    void set_flag(uint8_t flag);

    // sets the C flag
    void set_carry(bool carry);

    // sets the Z flag if the byte is 0
    void set_zero(uint8_t byte);

    // sets the N flag if bit 7 of the byte is set
    void set_negative(uint8_t byte);

    // sets the V flag based on the original value in the register,
    // the operand, and temporary resulting value.
    void set_overflow(uint8_t reg_value,
            uint8_t operand,
            uint16_t resulting_value);

    Pipeline parse_next_instruction();

    Pipeline create_branch_instruction(const std::function<bool()> &condition);
    Pipeline create_add_instruction(Opcode opcode);
    Pipeline create_and_instruction(Opcode opcode);
    Pipeline create_store_instruction(Opcode opcode);
    Pipeline create_load_instruction(Opcode opcode);
    Pipeline create_compare_instruction(Opcode opcode);

    Pipeline create_addressing_steps(AddressMode address_mode,
            bool is_write = false);

    Pipeline create_zeropage_addressing_steps();
    Pipeline create_zeropage_indexed_addressing_steps(const uint8_t *index_reg);
    Pipeline create_absolute_addressing_steps();
    Pipeline create_absolute_indexed_addressing_steps(const uint8_t *index_reg,
            bool is_write);
    Pipeline create_indexed_indirect_addressing_steps();
    Pipeline create_indirect_indexed_addressing_steps(bool is_write);
};

} // namespace n_e_s::core
