#include "cast_helpers.h"
#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuZeropageIndexedTest : public CpuTest {
public:
    enum class IndexReg { X, Y };

    void run_read_instruction(uint8_t instruction, IndexReg index_reg) {
        effective_address = u16_to_u8(index_value + reg_value);
        set_index_reg(index_reg, reg_value);
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1)).WillOnce(Return(index_value));
        EXPECT_CALL(mmu, read_byte(index_value))
                .WillOnce(Return(0xCD)); // Dummy read
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction(uint8_t instruction, IndexReg index_reg) {
        effective_address = u16_to_u8(index_value + reg_value);
        set_index_reg(index_reg, reg_value);
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1)).WillOnce(Return(index_value));
        EXPECT_CALL(mmu, read_byte(index_value))
                .WillOnce(Return(0xCD)); // Dummy read
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction(uint8_t instruction,
            IndexReg index_reg,
            uint8_t new_memory_content) {
        effective_address = u16_to_u8(index_value + reg_value);
        set_index_reg(index_reg, reg_value);
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1)).WillOnce(Return(index_value));
        EXPECT_CALL(mmu, read_byte(index_value))
                .WillOnce(Return(0xCD)); // Dummy read
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        EXPECT_CALL(mmu,
                write_byte(effective_address,
                        memory_content)); // Extra write with old value
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void load_sets_reg(uint8_t instruction,
            uint8_t *target_reg,
            IndexReg index_reg) {
        *target_reg = memory_content;
        run_read_instruction(instruction, index_reg);
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

    uint16_t start_pc{0x4321};
    uint8_t memory_content{0x39};
    uint8_t reg_value{0xED};
    uint8_t index_value{0x44};
    uint8_t effective_address{};
};

// ASL
TEST_F(CpuZeropageIndexedTest, asl_zerox_shifts) {
    memory_content = 0b00100101;
    run_readwrite_instruction(ASL_ZEROX, IndexReg::X, 0b01001010);
}

TEST_F(CpuZeropageIndexedTest, and_zerox_sets_neg) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG;
    expected.a = 0b10001010;
    expected.p = N_FLAG;
    memory_content = 0b10001111;

    run_read_instruction(AND_ZEROX, IndexReg::X);
}

TEST_F(CpuZeropageIndexedTest, lsr_zeropagex_sets_reg) {
    memory_content = 0b01001000;
    run_readwrite_instruction(LSR_ZEROX, IndexReg::X, 0b00100100);
}

TEST_F(CpuZeropageIndexedTest, adc_zero_x) {
    memory_content = 0x07;
    registers.a = 0x50;
    expected.a = 0x57;
    registers.p = V_FLAG;

    run_read_instruction(ADC_ZEROX, IndexReg::X);
}

TEST_F(CpuZeropageIndexedTest, sbc_zero_x) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x50 - 0x07;
    memory_content = 0x07;

    run_read_instruction(SBC_ZEROX, IndexReg::X);
}

TEST_F(CpuZeropageIndexedTest, isb_zero_x) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x50 - 0x07;
    memory_content = 0x07 - 0x01;

    run_readwrite_instruction(ISB_ZEROX, IndexReg::X, 0x07);
}

// LD Zeropage X
TEST_F(CpuZeropageIndexedTest, lda_zeropagex_sets_reg) {
    load_sets_reg(LDA_ZEROX, &expected.a, IndexReg::X);
}
TEST_F(CpuZeropageIndexedTest, ldy_zeropagex_sets_reg) {
    load_sets_reg(LDY_ZEROX, &expected.y, IndexReg::X);
}

// LD Zeropage Y
TEST_F(CpuZeropageIndexedTest, ldx_zeropagey_sets_reg) {
    load_sets_reg(LDX_ZEROY, &expected.x, IndexReg::Y);
}

TEST_F(CpuZeropageIndexedTest, lax_zeroy_sets_reg) {
    expected.x = 0x42;
    expected.a = 0x42;
    memory_content = 0x42;
    run_read_instruction(LAX_ZEROY, IndexReg::Y);
}

TEST_F(CpuZeropageIndexedTest, cmp_zero_x_sets_zc) {
    memory_content = 0x07;
    registers.a = expected.a = memory_content;
    registers.p |= N_FLAG;
    expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);

    run_read_instruction(CMP_ZEROX, IndexReg::X);
}

// DCP
TEST_F(CpuZeropageIndexedTest, dcp_decrements_sets_z_flag) {
    memory_content = 0x03;
    registers.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
    expected.a = registers.a = 0x01;
    expected.p = N_FLAG;
    run_readwrite_instruction(DCP_ZEROX, IndexReg::X, 0x02);
}

TEST_F(CpuZeropageIndexedTest, sta_zero_x_indexed) {
    registers.a = expected.a = memory_content;
    run_write_instruction(STA_ZEROX, IndexReg::X);
}

TEST_F(CpuZeropageIndexedTest, stx_zero_y_indexed) {
    memory_content = registers.x = expected.x = 0x07;
    run_write_instruction(STX_ZEROY, IndexReg::Y);
}

TEST_F(CpuZeropageIndexedTest, sty_zero_x_indexed) {
    memory_content = registers.y = expected.y = 0x07;
    run_write_instruction(STY_ZEROX, IndexReg::X);
}

// SAX
TEST_F(CpuZeropageIndexedTest, sax_zero_y_indexed) {
    registers.a = expected.a = 0b10101010;
    registers.x = expected.x = 0b00001111;
    memory_content = 0b00001010;
    run_write_instruction(SAX_ZEROY, IndexReg::Y);
}

TEST_F(CpuZeropageIndexedTest, eor_zero_x) {
    memory_content = 0x01;
    registers.a = 0b10000000;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0b10000001;
    expected.p = registers.p | N_FLAG;
    run_read_instruction(EOR_ZEROX, IndexReg::X);
}

// ROL
TEST_F(CpuZeropageIndexedTest, rol_zeropagex_sets_reg) {
    memory_content = 0b01001000;
    expected.p = N_FLAG;
    run_readwrite_instruction(ROL_ZEROX, IndexReg::X, 0b10010000);
}

TEST_F(CpuZeropageIndexedTest, ror_zeropagex_sets_reg) {
    memory_content = 0b01001000;
    run_readwrite_instruction(ROR_ZEROX, IndexReg::X, 0b00100100);
}

// ORA
TEST_F(CpuZeropageIndexedTest, ora_zero_x) {
    memory_content = 0b00100011;
    registers.a = 0b00110000;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00110011;
    run_read_instruction(ORA_ZEROX, IndexReg::X);
}

// SLO
TEST_F(CpuZeropageIndexedTest, slo_sets_z) {
    registers.a = 0b00000000;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b00000000;
    run_readwrite_instruction(SLO_ZEROX, IndexReg::X, 0b00000000);
}

// RLA
TEST_F(CpuZeropageIndexedTest, rla_sets_z) {
    registers.a = 0b00000000;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b00000000;
    run_readwrite_instruction(RLA_ZEROX, IndexReg::X, 0b00000000);
}

// SRE
TEST_F(CpuZeropageIndexedTest, sre_sets_z) {
    registers.a = 0b00000000;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b00000000;
    run_readwrite_instruction(SRE_ZEROX, IndexReg::X, 0b00000000);
}

// RRA
TEST_F(CpuZeropageIndexedTest, rra_sets_z) {
    registers.a = 0b0000'00000;
    expected.a = 0b0000'0000;
    expected.p = Z_FLAG;
    memory_content = 0b0000'00000;
    run_readwrite_instruction(RRA_ZEROX, IndexReg::X, 0b0000'0000);
}

} // namespace
