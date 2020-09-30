#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuIndirectIndexedTest : public CpuTest {
public:
    void run_read_instruction_without_pagecrossing(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        registers.y = expected.y = 0x10;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x42));
        EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

        const uint16_t effective_address = 0x1234 + registers.y;
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        step_execution(5);
        EXPECT_EQ(expected, registers);
    }

    void run_read_instruction_with_pagecrossing(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        registers.y = expected.y = 0xED;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x42));
        EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

        const uint16_t effective_address = 0x1234 + registers.y;
        EXPECT_CALL(mmu, read_byte(effective_address - 0x0100))
                .WillOnce(Return(0x00)); // Dummy read
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction_without_pagecrossing(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        registers.y = expected.y = 0x0D;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x42));
        EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

        const uint16_t effective_address = 0x1234 + registers.y;
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(0x00)); // Dummy read
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction_with_pagecrossing(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        registers.y = expected.y = 0xED;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x42));
        EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

        const uint16_t effective_address = 0x1234 + registers.y;
        EXPECT_CALL(mmu, read_byte(effective_address - 0x0100))
                .WillOnce(Return(0x00));
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction_without_pagecrossing(uint8_t instruction,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        registers.y = expected.y = 0x0D;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x42));
        EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

        const uint16_t effective_address = 0x1234 + 0x0D;
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(0xFF)); // Dummy read
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(8);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction_with_pagecrossing(uint8_t instruction,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        registers.y = expected.y = 0xD2;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1u)).WillOnce(Return(0x42));
        EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
        EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

        const uint16_t effective_address = 0x1234 + 0xD2;
        EXPECT_CALL(mmu, read_byte(effective_address - 0x0100))
                .WillOnce(Return(0xFF)); // Dummy read at wrong page
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(8);
        EXPECT_EQ(expected, registers);
    }

    uint16_t start_pc{0x4322};
    uint8_t memory_content{0x42};
};

// CMP Indirect indexed mode
TEST_F(CpuIndirectIndexedTest, cmp_with_pagecrossing_sets_nc) {
    registers.a = expected.a = 180;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;
    memory_content = 0x07;
    run_read_instruction_with_pagecrossing(CMP_INDINX);
}

TEST_F(CpuIndirectIndexedTest, cmp_without_pagecrossing_sets_nc) {
    registers.a = expected.a = 180;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;
    memory_content = 0x07;
    run_read_instruction_with_pagecrossing(CMP_INDINX);
}

// DCP
TEST_F(CpuIndirectIndexedTest, dcp_decrements_clears_ncz_without_pagecrossing) {
    memory_content = 0xFE;
    registers.p = static_cast<uint8_t>(Z_FLAG | C_FLAG) | N_FLAG;
    expected.a = registers.a = 0x02;
    run_readwrite_instruction_without_pagecrossing(DCP_INDINX, 0xFD);
}
TEST_F(CpuIndirectIndexedTest, dcp_decrements_clears_ncz_with_pagecrossing) {
    memory_content = 0xFE;
    registers.p = static_cast<uint8_t>(Z_FLAG | C_FLAG) | N_FLAG;
    expected.a = registers.a = 0x02;
    run_readwrite_instruction_with_pagecrossing(DCP_INDINX, 0xFD);
}

TEST_F(CpuIndirectIndexedTest, sta_indirect_indexed_with_pagecrossing) {
    registers.a = expected.a = 0x07;
    memory_content = 0x07;
    run_write_instruction_with_pagecrossing(STA_INDINX);
}

TEST_F(CpuIndirectIndexedTest, sta_indirect_indexed_without_pagecrossing) {
    registers.a = expected.a = 0x07;
    memory_content = 0x07;
    run_write_instruction_without_pagecrossing(STA_INDINX);
}

TEST_F(CpuIndirectIndexedTest, eor_indirect_indexed) {
    registers.a = 0x60;
    registers.p = C_FLAG;
    expected.a = 0x90;
    expected.p = N_FLAG | C_FLAG;
    memory_content = 0xF0;
    run_read_instruction_with_pagecrossing(EOR_INDINX);
}

// ORA, ABSX
TEST_F(CpuIndirectIndexedTest, ora_indirect_indexed) {
    registers.a = 0b00110000;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00110011;
    memory_content = 0b00100011;
    run_read_instruction_without_pagecrossing(ORA_INDINX);
}

TEST_F(CpuIndirectIndexedTest, and_indirect_indexed) {
    registers.a = 0b01110000;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b01110000;
    memory_content = 0b11111111;
    run_read_instruction_without_pagecrossing(AND_INDINX);
}

TEST_F(CpuIndirectIndexedTest, adc_indirect_indexed) {
    registers.a = 0x07;
    expected.a = 0x07 + 0x12;
    memory_content = 0x12;

    run_read_instruction_without_pagecrossing(ADC_INDY);
}

TEST_F(CpuIndirectIndexedTest, sbc_indirect_indexed) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;
    memory_content = 0xF0;
    run_read_instruction_without_pagecrossing(SBC_INDINX);
}

TEST_F(CpuIndirectIndexedTest, isb_indirect_indexed) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;
    memory_content = 0xF0 - 0x01;
    run_readwrite_instruction_without_pagecrossing(ISB_INDINX, 0xF0);
}

// SLO
TEST_F(CpuIndirectIndexedTest, slo) {
    registers.p = Z_FLAG;
    registers.a = 0b01110000;
    expected.a = 0b01110010;
    memory_content = 0b00000001;
    run_readwrite_instruction_without_pagecrossing(SLO_INDINX, 0b00000010);
}

// RLA
TEST_F(CpuIndirectIndexedTest, rla) {
    registers.p = Z_FLAG;
    registers.a = 0b01110111;
    expected.a = 0b00000010;
    memory_content = 0b00000001;
    run_readwrite_instruction_without_pagecrossing(RLA_INDINX, 0b00000010);
}

// SRE
TEST_F(CpuIndirectIndexedTest, sre) {
    registers.p = Z_FLAG;
    registers.a = 0b01110111;
    expected.a = 0b01110110;
    memory_content = 0b00000010;
    run_readwrite_instruction_without_pagecrossing(SRE_INDINX, 0b00000001);
}

// RRA
TEST_F(CpuIndirectIndexedTest, rra) {
    registers.p = Z_FLAG;
    registers.a = 0x07;
    expected.a = 0x07 + 0b0000'1010;
    memory_content = 0b0001'0100;
    run_readwrite_instruction_without_pagecrossing(RRA_INDINX, 0b0000'1010);
}

} // namespace
