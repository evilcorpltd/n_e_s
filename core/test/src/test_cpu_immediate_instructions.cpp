#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuImmediateTest : public CpuTest {
public:
    void run_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(memory_content));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void compare_sets_n_clears_z_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0b00000001;
        expected.p |= N_FLAG;
        registers.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
        memory_content = 0b10000001;

        run_instruction(instruction);
    }

    void compare_sets_c_clears_n_z(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0x1A;
        expected.p |= C_FLAG;
        registers.p |= static_cast<uint8_t>(Z_FLAG | N_FLAG);
        memory_content = 0x02;

        run_instruction(instruction);
    }

    void compare_sets_z_c_clears_n(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 42;
        expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
        registers.p |= N_FLAG;
        memory_content = 42;

        run_instruction(instruction);
    }

    void compare_sets_n_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 128;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        memory_content = 0;

        run_instruction(instruction);
    }

    void load_sets_reg(uint8_t instruction, uint8_t *target_reg) {
        memory_content = *target_reg = 99;
        run_instruction(instruction);
    }

    void load_sets_n_flag(uint8_t instruction, uint8_t *target_reg) {
        memory_content = *target_reg = 128;
        expected.p |= N_FLAG;
        run_instruction(instruction);
    }

    void load_clears_n_flag(uint8_t instruction, uint8_t *target_reg) {
        memory_content = *target_reg = 127;
        registers.p |= N_FLAG;
        run_instruction(instruction);
    }

    void load_sets_z_flag(uint8_t instruction, uint8_t *target_reg) {
        memory_content = *target_reg = 0;
        expected.p |= Z_FLAG;
        run_instruction(instruction);
    }

    void load_clears_z_flag(uint8_t instruction, uint8_t *target_reg) {
        memory_content = *target_reg = 1;
        registers.p |= Z_FLAG;
        run_instruction(instruction);
    }

    uint16_t start_pc{0x4321};
    uint8_t memory_content{0x42};
};

// AND
TEST_F(CpuImmediateTest, and) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_instruction(AND_IMM);
}

// ADC
TEST_F(CpuImmediateTest, adc_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    memory_content = 0x10;

    run_instruction(ADC_IMM);
}

TEST_F(CpuImmediateTest, adc_carry_but_no_overflow) {
    registers.a = 0xD0;
    registers.p = V_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x20;
    memory_content = 0x50;

    run_instruction(ADC_IMM);
}

TEST_F(CpuImmediateTest, adc_no_carry_but_overflow) {
    registers.a = 0x50;
    expected.p = V_FLAG | N_FLAG;
    expected.a = 0xA0;
    memory_content = 0x50;

    run_instruction(ADC_IMM);
}

TEST_F(CpuImmediateTest, adc_carry_and_overflow) {
    registers.a = 0xD0;
    expected.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0x90;

    run_instruction(ADC_IMM);
}

// SBC
TEST_F(CpuImmediateTest, sbc_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbc_carry_but_no_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x20;
    memory_content = 0x30;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbc_no_carry_but_overflow) {
    registers.a = 0x50;
    registers.p = C_FLAG;
    expected.p = V_FLAG | N_FLAG;
    expected.a = 0xA0;
    memory_content = 0xB0;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbc_carry_and_overflow) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0x70;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbceb_carry_and_overflow) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0x70;

    run_instruction(SBC_IMMEB);
}

// LDX
TEST_F(CpuImmediateTest, ldx_i_sets_reg) {
    load_sets_reg(LDX_IMM, &expected.x);
}
TEST_F(CpuImmediateTest, ldx_i_sets_n_flag) {
    load_sets_n_flag(LDX_IMM, &expected.x);
}
TEST_F(CpuImmediateTest, ldx_i_clears_n_flag) {
    load_clears_n_flag(LDX_IMM, &expected.x);
}
TEST_F(CpuImmediateTest, ldx_i_sets_z_flag) {
    load_sets_z_flag(LDX_IMM, &expected.x);
}
TEST_F(CpuImmediateTest, ldx_i_clears_z_flag) {
    load_clears_z_flag(LDX_IMM, &expected.x);
}

// LDY
TEST_F(CpuImmediateTest, ldy_i_sets_reg) {
    load_sets_reg(LDY_IMM, &expected.y);
}
TEST_F(CpuImmediateTest, ldy_i_sets_n_flag) {
    load_sets_n_flag(LDY_IMM, &expected.y);
}
TEST_F(CpuImmediateTest, ldy_i_clears_n_flag) {
    load_clears_n_flag(LDY_IMM, &expected.y);
}
TEST_F(CpuImmediateTest, ldy_i_sets_z_flag) {
    load_sets_z_flag(LDY_IMM, &expected.y);
}
TEST_F(CpuImmediateTest, ldy_i_clears_z_flag) {
    load_clears_z_flag(LDY_IMM, &expected.y);
}

// LDA
TEST_F(CpuImmediateTest, lda_i_sets_reg) {
    load_sets_reg(LDA_IMM, &expected.a);
}
TEST_F(CpuImmediateTest, lda_i_sets_n_flag) {
    load_sets_n_flag(LDA_IMM, &expected.a);
}
TEST_F(CpuImmediateTest, lda_i_clears_n_flag) {
    load_clears_n_flag(LDA_IMM, &expected.a);
}
TEST_F(CpuImmediateTest, lda_i_sets_z_flag) {
    load_sets_z_flag(LDA_IMM, &expected.a);
}
TEST_F(CpuImmediateTest, lda_i_clears_z_flag) {
    load_clears_z_flag(LDA_IMM, &expected.a);
}

// CPX
TEST_F(CpuImmediateTest, cpx_sets_n_and_clears_z_c) {
    compare_sets_n_clears_z_c(CPX_IMM, &registers.x, &expected.x);
}
TEST_F(CpuImmediateTest, cpx_sets_c_and_clears_n_z) {
    compare_sets_c_clears_n_z(CPX_IMM, &registers.x, &expected.x);
}
TEST_F(CpuImmediateTest, cpx_sets_z_c_clears_n) {
    compare_sets_z_c_clears_n(CPX_IMM, &registers.x, &expected.x);
}
TEST_F(CpuImmediateTest, cpx_sets_nc) {
    compare_sets_n_c(CPX_IMM, &registers.x, &expected.x);
}

// CPY
TEST_F(CpuImmediateTest, cpy_sets_n_and_clears_z_c) {
    compare_sets_n_clears_z_c(CPY_IMM, &registers.y, &expected.y);
}
TEST_F(CpuImmediateTest, cpy_sets_c_and_clears_n_z) {
    compare_sets_c_clears_n_z(CPY_IMM, &registers.y, &expected.y);
}
TEST_F(CpuImmediateTest, cpy_sets_z_c_clears_n) {
    compare_sets_z_c_clears_n(CPY_IMM, &registers.y, &expected.y);
}
TEST_F(CpuImmediateTest, cpy_sets_nc) {
    compare_sets_n_c(CPY_IMM, &registers.y, &expected.y);
}

// CMP
TEST_F(CpuImmediateTest, cmp_sets_n_and_clears_z_c) {
    compare_sets_n_clears_z_c(CMP_IMM, &registers.a, &expected.a);
}
TEST_F(CpuImmediateTest, cmp_sets_c_and_clears_n_z) {
    compare_sets_c_clears_n_z(CMP_IMM, &registers.a, &expected.a);
}
TEST_F(CpuImmediateTest, cmp_sets_z_c_clears_n) {
    compare_sets_z_c_clears_n(CMP_IMM, &registers.a, &expected.a);
}
TEST_F(CpuImmediateTest, cmp_sets_nc) {
    compare_sets_n_c(CMP_IMM, &registers.a, &expected.a);
}

// EOR
TEST_F(CpuImmediateTest, eor) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00111100;
    memory_content = 0b10010110;

    run_instruction(EOR_IMM);
}

// ORA
TEST_F(CpuImmediateTest, ora) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b11111111;
    expected.p = N_FLAG;
    memory_content = 0b01110101;

    run_instruction(ORA_IMM);
}

} // namespace
