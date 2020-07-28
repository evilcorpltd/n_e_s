#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuZeropageTest : public CpuTest {
public:
    void run_read_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(effective_address));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));

        step_execution(3);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(effective_address));
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));

        step_execution(3);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction(uint8_t instruction,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(effective_address));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        // Dummy write
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));
        step_execution(5);

        EXPECT_EQ(expected, registers);
    }

    void load_sets_reg(uint8_t instruction, uint8_t *target_reg) {
        *target_reg = 0x42;
        run_read_instruction(instruction);
    }

    void compare_sets_n_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 128;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        memory_content = 0;

        run_read_instruction(instruction);
    }

    uint16_t start_pc{0x4321};
    uint8_t memory_content{0x42};
    uint8_t effective_address{0x44};
};

TEST_F(CpuZeropageTest, asl_zero_clears_c_z_sets_n) {
    registers.p = C_FLAG | Z_FLAG;
    expected.p = N_FLAG;
    memory_content = 0b01010101;

    run_readwrite_instruction(ASL_ZERO, 0b10101010);
}

TEST_F(CpuZeropageTest, and) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_read_instruction(AND_ZERO);
}

TEST_F(CpuZeropageTest, bit_zero_sets_zero) {
    registers.a = expected.a = 0x00;
    expected.p = Z_FLAG;
    memory_content = 0x12;

    run_read_instruction(BIT_ZERO);
}
TEST_F(CpuZeropageTest, bit_zero_sets_negative) {
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;
    expected.p = N_FLAG;
    memory_content = 0xA3;

    run_read_instruction(BIT_ZERO);
}
TEST_F(CpuZeropageTest, bit_zero_sets_overflow) {
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;
    expected.p = V_FLAG;
    memory_content = 0x53;

    run_read_instruction(BIT_ZERO);
}

TEST_F(CpuZeropageTest, lsr_zeropage_shifts) {
    memory_content = 0b01001000;
    run_readwrite_instruction(LSR_ZERO, 0b00100100);
}

TEST_F(CpuZeropageTest, adc_zero_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    memory_content = 0x10;

    run_read_instruction(ADC_ZERO);
}

TEST_F(CpuZeropageTest, sbc_zero_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0;

    run_read_instruction(SBC_ZERO);
}

// ISB
TEST_F(CpuZeropageTest, isb_zero_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0 - 0x01;

    run_readwrite_instruction(ISB_ZERO, 0xF0);
}

// LD Zeropage
TEST_F(CpuZeropageTest, lda_zeropage_sets_reg) {
    load_sets_reg(LDA_ZERO, &expected.a);
}
TEST_F(CpuZeropageTest, ldx_zeropage_sets_reg) {
    load_sets_reg(LDX_ZERO, &expected.x);
}
TEST_F(CpuZeropageTest, ldy_zeropage_sets_reg) {
    load_sets_reg(LDY_ZERO, &expected.y);
}

// LAX
TEST_F(CpuZeropageTest, lax_zero_sets_reg_clear_flags) {
    registers.p = N_FLAG | Z_FLAG;
    expected.x = 0x42;
    expected.a = 0x42;
    memory_content = 0x42;
    run_read_instruction(LAX_ZERO);
}
TEST_F(CpuZeropageTest, lax_zero_sets_reg_sets_z) {
    expected.x = 0x00;
    expected.a = 0x00;
    memory_content = 0x00;
    expected.p |= Z_FLAG;
    run_read_instruction(LAX_ZERO);
}
TEST_F(CpuZeropageTest, lax_zero_sets_reg_sets_n) {
    expected.x = 0xF0;
    expected.a = 0xF0;
    memory_content = 0xF0;
    expected.p |= N_FLAG;
    run_read_instruction(LAX_ZERO);
}

// CPX, CPY, CMP Zeropage mode
TEST_F(CpuZeropageTest, cpx_zeropage_sets_nc) {
    compare_sets_n_c(CPX_ZERO, &registers.x, &expected.x);
}
TEST_F(CpuZeropageTest, cpy_zeropage_sets_nc) {
    compare_sets_n_c(CPY_ZERO, &registers.y, &expected.y);
}
TEST_F(CpuZeropageTest, cmp_zeropage_sets_nc) {
    compare_sets_n_c(CMP_ZERO, &registers.a, &expected.a);
}

// INC
TEST_F(CpuZeropageTest, inc_zero_increments) {
    memory_content = 0x05;
    run_readwrite_instruction(INC_ZERO, 0x06);
}
TEST_F(CpuZeropageTest, inc_zero_sets_z_flag) {
    expected.p = Z_FLAG;
    memory_content = 0xFF;
    run_readwrite_instruction(INC_ZERO, 0x00);
}
TEST_F(CpuZeropageTest, inc_zero_clears_z_flag_sets_n_flag) {
    registers.p = Z_FLAG;
    expected.p = N_FLAG;
    memory_content = 0xFD;
    run_readwrite_instruction(INC_ZERO, 0xFE);
}
TEST_F(CpuZeropageTest, inc_zero_clears_n_flag) {
    registers.p = N_FLAG;
    memory_content = 125;
    run_readwrite_instruction(INC_ZERO, 126);
}

// DEC
TEST_F(CpuZeropageTest, dec_zero_decrements) {
    memory_content = 0x06;
    run_readwrite_instruction(DEC_ZERO, 0x05);
}

TEST_F(CpuZeropageTest, dec_zero_sets_z_flag) {
    expected.p = Z_FLAG;
    memory_content = 0x01;
    run_readwrite_instruction(DEC_ZERO, 0x00);
}

TEST_F(CpuZeropageTest, dec_zero_clears_z_flag_sets_n_flag) {
    registers.p = Z_FLAG;
    expected.p = N_FLAG;
    memory_content = 0xFE;
    run_readwrite_instruction(DEC_ZERO, 0xFD);
}

TEST_F(CpuZeropageTest, dec_zero_clears_n_flag) {
    registers.p = N_FLAG;
    memory_content = 126;
    run_readwrite_instruction(DEC_ZERO, 125);
}

// DCP
TEST_F(CpuZeropageTest, dcp_decrements_sets_z_flag) {
    memory_content = 0x03;
    registers.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
    expected.a = registers.a = 0x01;
    expected.p = N_FLAG;
    run_readwrite_instruction(DCP_ZERO, 0x02);
}

TEST_F(CpuZeropageTest, sta_zero) {
    memory_content = registers.a = expected.a = 0x07;
    run_write_instruction(STA_ZERO);
}

TEST_F(CpuZeropageTest, stx_zero) {
    memory_content = registers.x = expected.x = 0x07;
    run_write_instruction(STX_ZERO);
}

TEST_F(CpuZeropageTest, sty_zero) {
    memory_content = registers.y = expected.y = 0x07;
    run_write_instruction(STY_ZERO);
}

// SAX
TEST_F(CpuZeropageTest, sax_zero) {
    registers.a = expected.a = 0b10101010;
    registers.x = expected.x = 0b00001111;
    memory_content = 0b00001010;
    run_write_instruction(SAX_ZERO);
}

TEST_F(CpuZeropageTest, eor_zero) {
    registers.a = 0xF0;
    expected.p = N_FLAG;
    expected.a = 0xFF;
    memory_content = 0x0F;

    run_read_instruction(EOR_ZERO);
}

// ROL
TEST_F(CpuZeropageTest, rol_zeropage_shifts) {
    memory_content = 0b01001000;
    expected.p = N_FLAG;
    run_readwrite_instruction(ROL_ZERO, 0b10010000);
}

TEST_F(CpuZeropageTest, ror_zeropage_shifts) {
    memory_content = 0b01001000;
    run_readwrite_instruction(ROR_ZERO, 0b00100100);
}

// ORA
TEST_F(CpuZeropageTest, ora_zero_set_neg_clears_zero) {
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b10101010;
    expected.p = N_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(ORA_ZERO);
}

// SLO
TEST_F(CpuZeropageTest, slo_zero_sets_n) {
    registers.p = C_FLAG | Z_FLAG;
    registers.a = 0b11110000;
    expected.a = 0b11111010;
    expected.p = N_FLAG;
    memory_content = 0b01010101;
    run_readwrite_instruction(SLO_ZERO, 0b10101010);
}

// RLA
TEST_F(CpuZeropageTest, rla_zero_sets_n_shifts_in_carry) {
    registers.p = C_FLAG | Z_FLAG;
    registers.a = 0b11111101;
    expected.a = 0b10101001;
    expected.p = N_FLAG;
    memory_content = 0b01010101;
    run_readwrite_instruction(RLA_ZERO, 0b10101011);
}

// SRE
TEST_F(CpuZeropageTest, sre_zero_sets_n) {
    registers.p = C_FLAG | Z_FLAG;
    registers.a = 0b11111101;
    expected.a = 0b11010111;
    expected.p = N_FLAG;
    memory_content = 0b01010100;
    run_readwrite_instruction(SRE_ZERO, 0b00101010);
}

} // namespace
