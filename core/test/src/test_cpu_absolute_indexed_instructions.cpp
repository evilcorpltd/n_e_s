#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuAbsoluteIndexedTest : public CpuTest {
public:
    enum class IndexReg { X, Y };
    void run_read_instruction_without_pagecrossing(uint8_t instruction,
            IndexReg index_reg) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const uint8_t index_reg_value{0x10};
        set_index_reg(index_reg, index_reg_value);

        const uint16_t effective_address = 0x5678 + index_reg_value;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x78));
        EXPECT_CALL(mmu, read_byte(start_pc + 2u)).WillOnce(Return(0x56));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_read_instruction_with_pagecrossing(uint8_t instruction,
            IndexReg index_reg) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const uint8_t index_reg_value{0xAB};
        set_index_reg(index_reg, index_reg_value);

        const uint16_t effective_address = 0x5678 + index_reg_value;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x78));
        EXPECT_CALL(mmu, read_byte(start_pc + 2u)).WillOnce(Return(0x56));
        EXPECT_CALL(mmu, read_byte(effective_address - 0x0100))
                .WillOnce(Return(0xCD)); // Extra read
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));

        step_execution(5);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction_without_pagecrossing(uint8_t instruction,
            IndexReg index_reg) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const uint8_t index_reg_value{0x10};
        set_index_reg(index_reg, index_reg_value);

        const uint16_t effective_address = 0x5678 + index_reg_value;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x78));
        EXPECT_CALL(mmu, read_byte(start_pc + 2u)).WillOnce(Return(0x56));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(0xCD)); // Extra read
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));

        step_execution(5);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction_with_pagecrossing(uint8_t instruction,
            IndexReg index_reg) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const uint8_t index_reg_value{0xAB};
        set_index_reg(index_reg, index_reg_value);

        const uint16_t effective_address = 0x5678 + index_reg_value;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x78));
        EXPECT_CALL(mmu, read_byte(start_pc + 2u)).WillOnce(Return(0x56));
        EXPECT_CALL(mmu, read_byte(effective_address - 0x0100))
                .WillOnce(Return(0xCD)); // Extra read
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));

        step_execution(5);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction_without_pagecrossing(uint8_t instruction,
            IndexReg index_reg,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const uint8_t index_reg_value{0x21};
        set_index_reg(index_reg, index_reg_value);

        const uint16_t effective_address = 0x5678 + index_reg_value;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x78));
        EXPECT_CALL(mmu, read_byte(start_pc + 2u)).WillOnce(Return(0x56));
        EXPECT_CALL(mmu, read_byte(0x5699))
                .WillOnce(Return(0xCD)); // Dummy read
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        EXPECT_CALL(mmu,
                write_byte(effective_address, memory_content)); // Dummy write
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(7);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction_with_pagecrossing(uint8_t instruction,
            IndexReg index_reg,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const uint8_t index_reg_value{0x91};
        set_index_reg(index_reg, index_reg_value);

        const uint16_t effective_address = 0x5678 + index_reg_value;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x78));
        EXPECT_CALL(mmu, read_byte(start_pc + 2u)).WillOnce(Return(0x56));
        EXPECT_CALL(mmu, read_byte(0x5609))
                .WillOnce(Return(
                        0xCD)); // Dummy read with on wrong page (0x5678 + 0x91)
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        EXPECT_CALL(mmu,
                write_byte(effective_address, memory_content)); // Dummy write
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(7);
        EXPECT_EQ(expected, registers);
    }

    void set_index_reg(const IndexReg index_reg, const uint8_t value) {
        if (index_reg == IndexReg::X) {
            registers.x = value;
            expected.x = value;
        } else {
            registers.y = value;
            expected.y = value;
        }
    }

    uint16_t start_pc{0x1121};
    uint8_t memory_content{0x42};
};

// ASL
TEST_F(CpuAbsoluteIndexedTest, asl_absx_shifts_without_pagecrossing) {
    memory_content = 0b00100101;
    run_readwrite_instruction_without_pagecrossing(
            ASL_ABSX, IndexReg::X, 0b01001010);
}
TEST_F(CpuAbsoluteIndexedTest, asl_absx_shifts_with_pagecrossing) {
    memory_content = 0b00100101;
    run_readwrite_instruction_with_pagecrossing(
            ASL_ABSX, IndexReg::X, 0b01001010);
}

TEST_F(CpuAbsoluteIndexedTest, and_absx_without_page_crossing) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_read_instruction_without_pagecrossing(AND_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, and_absx_with_page_crossing) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_read_instruction_with_pagecrossing(AND_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, and_absy_without_page_crossing) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_read_instruction_without_pagecrossing(AND_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, and_absy_with_page_crossing) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_read_instruction_with_pagecrossing(AND_ABSY, IndexReg::Y);
}

TEST_F(CpuAbsoluteIndexedTest, adc_absx_no_carry_or_overflow_no_pagecrossing) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x71;
    memory_content = 0x21;

    run_read_instruction_with_pagecrossing(ADC_ABSX, IndexReg::X);
}

TEST_F(CpuAbsoluteIndexedTest,
        adc_absy_no_carry_or_overflow_with_pagecrossing) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x71;
    memory_content = 0x21;

    run_read_instruction_with_pagecrossing(ADC_ABSY, IndexReg::Y);
}

TEST_F(CpuAbsoluteIndexedTest, sbc_absx_no_carry_or_overflow_no_pagecrossing) {
    registers.a = 0xD0;
    registers.p = V_FLAG | C_FLAG;
    expected.p = N_FLAG;
    expected.a = 0xE0;
    memory_content = 0xF0;

    run_read_instruction_without_pagecrossing(SBC_ABSX, IndexReg::X);
}

TEST_F(CpuAbsoluteIndexedTest,
        sbc_absy_no_carry_or_overflow_with_pagecrossing) {
    registers.a = 0xD0;
    registers.p = V_FLAG | C_FLAG;
    expected.p = N_FLAG;
    expected.a = 0xE0;
    memory_content = 0xF0;

    run_read_instruction_with_pagecrossing(SBC_ABSX, IndexReg::X);
}

// LD absolute indexed
// LDA_ABSY
TEST_F(CpuAbsoluteIndexedTest, lda_abs_y_sets_reg) {
    expected.a = memory_content = 0x37;
    run_read_instruction_without_pagecrossing(LDA_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, lda_abs_y_sets_reg_crossing_page) {
    expected.a = memory_content = 0x78;
    run_read_instruction_with_pagecrossing(LDA_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, lda_abs_y_sets_z_flag) {
    expected.a = memory_content = 0;
    expected.p |= Z_FLAG;
    registers.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDA_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, lda_abs_y_sets_n) {
    expected.a = memory_content = 0xA7;
    expected.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDA_ABSY, IndexReg::Y);
}
// LDY_ABSX
TEST_F(CpuAbsoluteIndexedTest, ldy_abs_x_sets_reg) {
    expected.y = memory_content = 0x37;
    run_read_instruction_without_pagecrossing(LDY_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, ldy_abs_x_sets_reg_crossing_page) {
    expected.y = memory_content = 0x78;
    run_read_instruction_with_pagecrossing(LDY_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, ldy_abs_x_sets_z_flag) {
    expected.y = memory_content = 0;
    expected.p |= Z_FLAG;
    registers.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDY_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, ldy_abs_x_sets_n) {
    expected.y = memory_content = 0xA7;
    expected.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDY_ABSX, IndexReg::X);
}
// LDA_ABSX
TEST_F(CpuAbsoluteIndexedTest, lda_abs_x_sets_reg) {
    expected.a = memory_content = 0x37;
    run_read_instruction_without_pagecrossing(LDA_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, lda_abs_x_sets_reg_crossing_page) {
    expected.a = memory_content = 0x78;
    run_read_instruction_with_pagecrossing(LDA_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, lda_abs_x_sets_z_flag) {
    expected.a = memory_content = 0;
    expected.p |= Z_FLAG;
    registers.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDA_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, lda_abs_x_sets_n) {
    expected.a = memory_content = 0xA7;
    expected.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDA_ABSX, IndexReg::X);
}
// LDX_ABSY
TEST_F(CpuAbsoluteIndexedTest, ldx_abs_y_sets_reg) {
    expected.x = memory_content = 0x37;
    run_read_instruction_without_pagecrossing(LDX_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, ldx_abs_y_sets_reg_crossing_page) {
    expected.x = memory_content = 0x78;
    run_read_instruction_with_pagecrossing(LDX_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, ldx_abs_y_sets_z_flag) {
    expected.x = memory_content = 0;
    expected.p |= Z_FLAG;
    registers.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDX_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, ldx_abs_y_sets_n) {
    expected.x = memory_content = 0xA7;
    expected.p |= N_FLAG;
    run_read_instruction_without_pagecrossing(LDX_ABSY, IndexReg::Y);
}

// LAX
TEST_F(CpuAbsoluteIndexedTest, lax_absy_sets_reg_without_pagecrossing) {
    expected.x = 0x42;
    expected.a = 0x42;
    memory_content = 0x42;
    run_read_instruction_without_pagecrossing(LAX_ABSY, IndexReg::Y);
}
TEST_F(CpuAbsoluteIndexedTest, lax_absy_sets_reg_with_pagecrossing) {
    expected.x = 0x42;
    expected.a = 0x42;
    memory_content = 0x42;
    run_read_instruction_with_pagecrossing(LAX_ABSY, IndexReg::Y);
}

// CMP Absolute indexed mode
TEST_F(CpuAbsoluteIndexedTest, cmp_absx_sets_zc_without_pagecrossing) {
    memory_content = registers.a = expected.a = 0x07;
    registers.p |= N_FLAG;
    expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
    run_read_instruction_without_pagecrossing(CMP_ABSX, IndexReg::X);
}

TEST_F(CpuAbsoluteIndexedTest, cmp_absx_sets_zc_with_pagecrossing) {
    memory_content = registers.a = expected.a = 0x07;
    registers.p |= N_FLAG;
    expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
    run_read_instruction_with_pagecrossing(CMP_ABSX, IndexReg::X);
}

TEST_F(CpuAbsoluteIndexedTest, cmp_absy_sets_zc_without_pagecrossing) {
    memory_content = registers.a = expected.a = 0x07;
    registers.p |= N_FLAG;
    expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
    run_read_instruction_without_pagecrossing(CMP_ABSY, IndexReg::Y);
}

TEST_F(CpuAbsoluteIndexedTest, cmp_absy_sets_zc_with_pagecrossing) {
    memory_content = registers.a = expected.a = 0x07;
    registers.p |= N_FLAG;
    expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
    run_read_instruction_with_pagecrossing(CMP_ABSY, IndexReg::Y);
}

TEST_F(CpuAbsoluteIndexedTest, inc_absx_clears_n_flag) {
    registers.p = N_FLAG;
    memory_content = 125u;
    run_readwrite_instruction_with_pagecrossing(INC_ABSX, IndexReg::X, 126u);
}

// DEC
TEST_F(CpuAbsoluteIndexedTest, dec_absx_clears_n_flag) {
    registers.p = N_FLAG;
    memory_content = 126;
    run_readwrite_instruction_with_pagecrossing(DEC_ABSX, IndexReg::X, 125);
}

// DCP
TEST_F(CpuAbsoluteIndexedTest, dcp_absx_sets_n_flag) {
    memory_content = 126;
    expected.a = registers.a = 0x01;
    expected.p = N_FLAG;
    run_readwrite_instruction_with_pagecrossing(DCP_ABSX, IndexReg::X, 125);
}
TEST_F(CpuAbsoluteIndexedTest, dcp_absy_sets_n_flag) {
    memory_content = 126;
    expected.a = registers.a = 0x01;
    expected.p = N_FLAG;
    run_readwrite_instruction_with_pagecrossing(DCP_ABSY, IndexReg::Y, 125);
}

// ORA, ABSY
TEST_F(CpuAbsoluteIndexedTest, ora_absy_without_page_crossing) {
    registers.a = 0b00111100;
    expected.a = 0b11111100;
    expected.p = N_FLAG;
    memory_content = 0b11110000;

    run_read_instruction_without_pagecrossing(ORA_ABSY, IndexReg::Y);
}

// ORA, ABSX
TEST_F(CpuAbsoluteIndexedTest, ora_absx_without_page_crossing) {
    registers.a = 0b00110000;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00110011;
    memory_content = 0b00100011;

    run_read_instruction_without_pagecrossing(ORA_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, ora_absx_with_page_crossing) {
    registers.a = 0b10000000;
    registers.p = N_FLAG;
    expected.a = 0b10000001;
    expected.p = N_FLAG;
    memory_content = 0b00000001;

    run_read_instruction_with_pagecrossing(ORA_ABSX, IndexReg::X);
}

// EOR
TEST_F(CpuAbsoluteIndexedTest, eor_absx_with_page_crossing) {
    registers.a = 0b11110000;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b11110000;

    run_read_instruction_with_pagecrossing(EOR_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, eor_absx_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b11111100;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00110011;
    memory_content = 0b11001111;

    run_read_instruction_without_pagecrossing(EOR_ABSX, IndexReg::X);
}
TEST_F(CpuAbsoluteIndexedTest, eor_absy_without_page_crossing) {
    registers.a = 0b00111100;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b10011001;
    expected.p = N_FLAG;
    memory_content = 0b10100101;

    run_read_instruction_without_pagecrossing(EOR_ABSY, IndexReg::Y);
}

// ROL
TEST_F(CpuAbsoluteIndexedTest, rol_absx_shifts_without_pagecrossing) {
    expected.p = N_FLAG;
    memory_content = 0b01001000;

    run_readwrite_instruction_without_pagecrossing(
            ROL_ABSX, IndexReg::X, 0b10010000);
}
TEST_F(CpuAbsoluteIndexedTest, rol_absx_shifts_with_pagecrossing) {
    expected.p = N_FLAG;
    memory_content = 0b01001000;

    run_readwrite_instruction_with_pagecrossing(
            ROL_ABSX, IndexReg::X, 0b10010000);
}

// ROR
TEST_F(CpuAbsoluteIndexedTest, ror_absx_shifts_without_pagecrossing) {
    memory_content = 0b01001000;

    run_readwrite_instruction_without_pagecrossing(
            ROR_ABSX, IndexReg::X, 0b00100100);
}
TEST_F(CpuAbsoluteIndexedTest, ror_absx_shifts_with_pagecrossing) {
    memory_content = 0b01001000;

    run_readwrite_instruction_with_pagecrossing(
            ROR_ABSX, IndexReg::X, 0b00100100);
}

// LSR
TEST_F(CpuAbsoluteIndexedTest, lsr_absx_shifts_without_pagecrossing) {
    memory_content = 0b01001000;

    run_readwrite_instruction_without_pagecrossing(
            LSR_ABSX, IndexReg::X, 0b00100100);
}

TEST_F(CpuAbsoluteIndexedTest, sta_abs_x_indexed_with_pagecrossing) {
    registers.a = expected.a = 0x07;
    memory_content = 0x07;

    run_write_instruction_with_pagecrossing(STA_ABSX, IndexReg::X);
}

TEST_F(CpuAbsoluteIndexedTest, sta_abs_y_indexed_without_pagecrossing) {
    registers.a = expected.a = 0x07;
    memory_content = 0x07;

    run_write_instruction_without_pagecrossing(STA_ABSY, IndexReg::Y);
}

} // namespace
