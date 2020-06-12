#include "cast_helpers.h"
#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuAbsoluteTest : public CpuTest {
public:
    void run_read_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const auto lower_address = static_cast<uint8_t>(
                effective_address & static_cast<uint16_t>(0x00FFu));
        const auto upper_address = static_cast<uint8_t>(
                static_cast<uint16_t>(effective_address & 0xFF00u) >> 8u);

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(lower_address));
        EXPECT_CALL(mmu, read_byte(start_pc + 2))
                .WillOnce(Return(upper_address));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const auto lower_address = static_cast<uint8_t>(
                effective_address & static_cast<uint16_t>(0x00FFu));
        const auto upper_address =
                static_cast<uint8_t>((effective_address & 0xFF00u) >> 8u);

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(lower_address));
        EXPECT_CALL(mmu, read_byte(start_pc + 2))
                .WillOnce(Return(upper_address));
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction(uint8_t instruction,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const auto lower_address = static_cast<uint8_t>(
                effective_address & static_cast<uint16_t>(0x00FFu));
        const auto upper_address =
                static_cast<uint8_t>((effective_address & 0xFF00u) >> 8u);

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(lower_address));
        EXPECT_CALL(mmu, read_byte(start_pc + 2))
                .WillOnce(Return(upper_address));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        // Dummy write
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void compare_abs_sets_n_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 128;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        memory_content = 0;
        run_read_instruction(instruction);
    }

    void load_absolute_sets_reg(uint8_t instruction, uint8_t *reg) {
        *reg = 0x42;
        memory_content = *reg;
        run_read_instruction(instruction);
    }

    void load_absolute_sets_n_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 128;
        memory_content = *reg;
        expected.p |= N_FLAG;
        run_read_instruction(instruction);
    }

    void load_absolute_clears_n_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 127;
        memory_content = *reg;
        registers.p |= N_FLAG;
        run_read_instruction(instruction);
    }

    void load_absolute_sets_z_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 0;
        memory_content = *reg;
        expected.p |= Z_FLAG;
        run_read_instruction(instruction);
    }

    void load_absolute_clears_z_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 1;
        memory_content = *reg;
        registers.p |= Z_FLAG;
        run_read_instruction(instruction);
    }

    uint16_t start_pc{0x1121};
    uint8_t memory_content{0x42};
    uint16_t effective_address{0x4567};
};

// ASL
TEST_F(CpuAbsoluteTest, asl_abs_sets_z_c) {
    registers.p = N_FLAG;
    expected.p = C_FLAG | Z_FLAG;
    memory_content = 0b10000000;

    run_readwrite_instruction(ASL_ABS, 0b00000000);
}

TEST_F(CpuAbsoluteTest, and_abs_sets_zero_clears_neg) {
    registers.a = 0b10101010;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b01010101;

    run_read_instruction(AND_ABS);
}

TEST_F(CpuAbsoluteTest, and_abs_sets_neg_clears_zero) {
    registers.a = 0b11000011;
    registers.p = Z_FLAG;
    expected.a = 0b11000001;
    expected.p = N_FLAG;
    memory_content = 0b11110001;

    run_read_instruction(AND_ABS);
}

TEST_F(CpuAbsoluteTest, bit_abs_sets_negative_and_overflow) {
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;
    expected.p = V_FLAG | N_FLAG;
    memory_content = 0xC3;

    run_read_instruction(BIT_ABS);
}

TEST_F(CpuAbsoluteTest, lsr_abs_shifts) {
    memory_content = 0b01001000;
    run_readwrite_instruction(LSR_ABS, 0b00100100);
}
TEST_F(CpuAbsoluteTest, lsr_abs_shifts_in_zero) {
    memory_content = 0b01001000;
    registers.p = C_FLAG;
    run_readwrite_instruction(LSR_ABS, 0b00100100);
}
TEST_F(CpuAbsoluteTest, lsr_abs_shifts_out_carry) {
    memory_content = 0b01001001;
    expected.p = C_FLAG;
    run_readwrite_instruction(LSR_ABS, 0b00100100);
}

TEST_F(CpuAbsoluteTest, adc_abs_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    memory_content = 0x10;

    run_read_instruction(ADC_ABS);
}

TEST_F(CpuAbsoluteTest, sbc_abs_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0;

    run_read_instruction(SBC_ABS);
}

TEST_F(CpuAbsoluteTest, isb_abs_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0 - 0x01;

    run_readwrite_instruction(ISB_ABS, 0xF0);
}

// LDX Absolute mode
TEST_F(CpuAbsoluteTest, ldx_abs_sets_reg) {
    load_absolute_sets_reg(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_sets_n_flag) {
    load_absolute_sets_n_flag(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_clears_n_flag) {
    load_absolute_clears_n_flag(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_sets_z_flag) {
    load_absolute_sets_z_flag(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_clears_z_flag) {
    load_absolute_clears_z_flag(LDX_ABS, &expected.x);
}

// LDY Absolute mode
TEST_F(CpuAbsoluteTest, ldy_abs_sets_reg) {
    load_absolute_sets_reg(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_sets_n_flag) {
    load_absolute_sets_n_flag(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_clears_n_flag) {
    load_absolute_clears_n_flag(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_sets_z_flag) {
    load_absolute_sets_z_flag(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_clears_z_flag) {
    load_absolute_clears_z_flag(LDY_ABS, &expected.y);
}

// LDA Absolute mode
TEST_F(CpuAbsoluteTest, lda_abs_sets_reg) {
    load_absolute_sets_reg(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_sets_n_flag) {
    load_absolute_sets_n_flag(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_clears_n_flag) {
    load_absolute_clears_n_flag(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_sets_z_flag) {
    load_absolute_sets_z_flag(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_clears_z_flag) {
    load_absolute_clears_z_flag(LDA_ABS, &expected.a);
}

// LAX
TEST_F(CpuAbsoluteTest, lax_abs_sets_reg) {
    expected.x = 0x42;
    expected.a = 0x42;
    memory_content = 0x42;
    run_read_instruction(LAX_ABS);
}

// CPX, CPY, CMP
TEST_F(CpuAbsoluteTest, cpx_abs_sets_nc) {
    compare_abs_sets_n_c(CPX_ABS, &registers.x, &expected.x);
}
TEST_F(CpuAbsoluteTest, cpy_abs_sets_nc) {
    compare_abs_sets_n_c(CPY_ABS, &registers.y, &expected.y);
}
TEST_F(CpuAbsoluteTest, cmp_abs_sets_nc) {
    compare_abs_sets_n_c(CMP_ABS, &registers.a, &expected.a);
}

TEST_F(CpuAbsoluteTest, inc_abs_increments) {
    memory_content = 0x09;
    run_readwrite_instruction(INC_ABS, 0x0A);
}

// DEC
TEST_F(CpuAbsoluteTest, dec_abs_decrements) {
    memory_content = 0x09;
    run_readwrite_instruction(DEC_ABS, 0x08);
}

// DCP
TEST_F(CpuAbsoluteTest, dcp_decrements_sets_c_flag) {
    memory_content = 0x02;
    registers.p |= static_cast<uint8_t>(Z_FLAG | N_FLAG);
    expected.a = registers.a = 0x1A;
    expected.p = C_FLAG;
    run_readwrite_instruction(DCP_ABS, 0x01);
}

// STA
TEST_F(CpuAbsoluteTest, sta_abs) {
    memory_content = registers.a = expected.a = 0x45;
    run_write_instruction(STA_ABS);
}

TEST_F(CpuAbsoluteTest, stx_abs) {
    memory_content = registers.x = expected.x = 0x71;
    run_write_instruction(STX_ABS);
}

TEST_F(CpuAbsoluteTest, sty_abs) {
    memory_content = registers.y = expected.y = 0x07;
    run_write_instruction(STY_ABS);
}

// SAX
TEST_F(CpuAbsoluteTest, sax_abs) {
    registers.a = expected.a = 0b10101010;
    registers.x = expected.x = 0b11110000;
    memory_content = 0b10100000;
    run_write_instruction(SAX_ABS);
}

TEST_F(CpuAbsoluteTest, eor_abs_set_zero_clears_neg) {
    registers.a = 0b10101010;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(EOR_ABS);
}

TEST_F(CpuAbsoluteTest, eor_abs_set_neg_clears_zero) {
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b10101010;
    expected.p = N_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(EOR_ABS);
}

// ROL
TEST_F(CpuAbsoluteTest, rol_abs_shifts) {
    memory_content = 0b01001000;
    expected.p = N_FLAG;
    run_readwrite_instruction(ROL_ABS, 0b10010000);
}
TEST_F(CpuAbsoluteTest, rol_abs_shifts_in_carry) {
    memory_content = 0b01001000;
    registers.p = C_FLAG;
    expected.p = N_FLAG;
    run_readwrite_instruction(ROL_ABS, 0b10010001);
}
TEST_F(CpuAbsoluteTest, rol_abs_shifts_out_carry) {
    memory_content = 0b10001001;
    expected.p = C_FLAG;
    run_readwrite_instruction(ROL_ABS, 0b00010010);
}

TEST_F(CpuAbsoluteTest, ror_abs_shifts) {
    memory_content = 0b01001000;
    run_readwrite_instruction(ROR_ABS, 0b00100100);
}
TEST_F(CpuAbsoluteTest, ror_abs_shifts_in_carry) {
    memory_content = 0b01001000;
    registers.p = C_FLAG;
    expected.p = N_FLAG;
    run_readwrite_instruction(ROR_ABS, 0b10100100);
}
TEST_F(CpuAbsoluteTest, ror_abs_shifts_out_carry) {
    memory_content = 0b01001001;
    expected.p = C_FLAG;
    run_readwrite_instruction(ROR_ABS, 0b00100100);
}

// ORA
TEST_F(CpuAbsoluteTest, ora_abs_set_neg_clears_zero) {
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b10101010;
    expected.p = N_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(ORA_ABS);
}

// SLO
TEST_F(CpuAbsoluteTest, slo_sets_c_and_n) {
    registers.p = Z_FLAG;
    registers.a = 0b11110000;
    expected.a = 0b11110010;
    expected.p = C_FLAG | N_FLAG;
    memory_content = 0b10000001;

    run_readwrite_instruction(SLO_ABS, 0b00000010);
}

} // namespace
