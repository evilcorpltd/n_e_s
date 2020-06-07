#include "cast_helpers.h"
#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuIndexedIndirectTest : public CpuTest {
public:
    void run_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        registers.x = expected.x = 0xED;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0xAB));
        EXPECT_CALL(mmu, read_byte(0xAB)).WillOnce(Return(0x68)); // Dummy read
        EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED)))
                .WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED + 1u)))
                .WillOnce(Return(0x12));
        EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(memory_content));

        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void run_instruction_with_wraparound(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        registers.x = expected.x = 0x00;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0xFF));
        EXPECT_CALL(mmu, read_byte(0xFF)).WillOnce(Return(0x68)); // Dummy read
        EXPECT_CALL(mmu, read_byte(0xFF)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x00)).WillOnce(Return(0x12));
        EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(memory_content));

        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction(uint8_t instruction,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        registers.x = expected.x = 0xED;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0xAB));
        EXPECT_CALL(mmu, read_byte(0xAB)).WillOnce(Return(0x68)); // Dummy read
        EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED)))
                .WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED + 1u)))
                .WillOnce(Return(0x12));
        EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(memory_content));
        EXPECT_CALL(mmu, write_byte(0x1234, memory_content));
        EXPECT_CALL(mmu, write_byte(0x1234, new_memory_content));

        step_execution(8);
        EXPECT_EQ(expected, registers);
    }

    uint16_t start_pc{0x4322};
    uint8_t memory_content{0x42};
};

TEST_F(CpuIndexedIndirectTest, and) {
    memory_content = 0b00001111;
    registers.a = 0b11101101;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001101;

    run_instruction(AND_INXIND);
}

TEST_F(CpuIndexedIndirectTest, adc_indexed_indirect) {
    registers.a = 0x07;
    expected.a = 0x07 + 0x12;
    memory_content = 0x12;

    run_instruction(ADC_INDX);
}

TEST_F(CpuIndexedIndirectTest, sbc) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;
    memory_content = 0xF0;

    run_instruction(SBC_INXIND);
}
TEST_F(CpuIndexedIndirectTest, sbc_handles_wraparound) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;
    memory_content = 0xF0;

    run_instruction_with_wraparound(SBC_INXIND);
}

// LDA indexed indirect
TEST_F(CpuIndexedIndirectTest, lda_indexed_indirect) {
    expected.a = 0x52;
    memory_content = 0x52;

    run_instruction(LDA_INXIND);
}

TEST_F(CpuIndexedIndirectTest, cmp_indexed_indirect_sets_nc) {
    registers.a = expected.a = 180;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;
    memory_content = 0x07;

    run_instruction(CMP_INXIND);
}
TEST_F(CpuIndexedIndirectTest,
        cmp_indexed_indirect_handles_wraparound_sets_nc) {
    registers.a = expected.a = 180;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;
    memory_content = 0x07;

    run_instruction_with_wraparound(CMP_INXIND);
}

// DCP
TEST_F(CpuIndexedIndirectTest, dcp_decrements_clears_n) {
    memory_content = 0x12;
    registers.p = N_FLAG;
    expected.p = C_FLAG;
    expected.a = registers.a = 0x42;
    run_readwrite_instruction(DCP_INXIND, 0x11);
}

TEST_F(CpuIndexedIndirectTest, eor_indexed_indirect) {
    registers.a = 0xE0;
    expected.a = 0x00;
    expected.p = Z_FLAG;
    memory_content = 0xE0;
    run_instruction(EOR_INXIND);
}

// ORA, ABSX
TEST_F(CpuIndexedIndirectTest, ora) {
    memory_content = 0b00100011;
    registers.a = 0b00110000;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00110011;
    run_instruction(ORA_INXIND);
}

// ISB
TEST_F(CpuIndexedIndirectTest, isb_zero_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0 - 0x01;

    run_readwrite_instruction(ISB_INXIND, 0xF0);
}

} // namespace
