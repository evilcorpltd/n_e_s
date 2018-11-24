// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/cpu_factory.h"

#include "mock_mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace n_e_s::core {

static bool operator==(const Registers &a, const Registers &b) {
    return a.pc == b.pc && a.sp == b.sp && a.a == b.a && a.x == b.x &&
           a.y == b.y && a.p == b.p;
}

} // namespace n_e_s::core

namespace {

const uint16_t kStackOffset = 0x0100;
const uint16_t kResetAddress = 0xFFFC;
const uint16_t kBrkAddress = 0xFFFE;

// Tests and opcodes should be written without looking at the cpu
// implementation. Look at a data sheet and don't cheat!
enum Opcode : uint8_t {
    BRK = 0x00,
    PHP = 0x08,
    CLC = 0x18,
    SEC = 0x38,
    LSR_A = 0x4A,
    PHA = 0x48,
    JMP = 0x4C,
    CLI = 0x58,
    SEI = 0x78,
    LDY_I = 0xA0,
    CLV = 0xB8,
    CLD = 0xD8,
    NOP = 0xEA,
    INX = 0xE8,
    SED = 0xF8,
};

class CpuTest : public ::testing::Test {
public:
    CpuTest()
            : registers(),
              mmu(),
              cpu{CpuFactory::create(&registers, &mmu)},
              expected() {}

    void stage_instruction(uint8_t instruction) {
        expected.pc += 1;
        ON_CALL(mmu, read_byte(registers.pc))
                .WillByDefault(Return(instruction));
    }

    void step_execution(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            cpu->execute();
        }
    }

    Registers registers;
    NiceMock<MockMmu> mmu;
    std::unique_ptr<ICpu> cpu;

    Registers expected;
};

TEST_F(CpuTest, reset) {
    expected.pc = 0xDEAD;
    ON_CALL(mmu, read_word(kResetAddress)).WillByDefault(Return(0xDEAD));

    cpu->reset();

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, reset_clears_pipeline) {
    stage_instruction(SEC);
    ON_CALL(mmu, read_word(kResetAddress)).WillByDefault(Return(0xDEAD));
    expected.pc = 0xDEAD + 1;

    cpu->execute(); // Stage things for execution.
    cpu->reset();
    cpu->execute(); // Should read an opcode and not execute what's been staged.

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, unsupported_instruction) {
    stage_instruction(0xFF);

    EXPECT_THROW(step_execution(1), std::logic_error);
}

TEST_F(CpuTest, brk) {
    stage_instruction(BRK);
    expected.pc = 0xDEAD;

    const uint8_t pc_size = 2;
    const uint8_t expected_pc_stack_addr = expected.sp - pc_size;
    const uint8_t p_size = 1;
    const uint8_t expected_p_stack_addr = expected_pc_stack_addr - p_size;

    expected.sp -= pc_size + p_size;

    ON_CALL(mmu, read_word(kBrkAddress)).WillByDefault(Return(0xDEAD));

    // First the return address is pushed and then the registers.
    EXPECT_CALL(mmu, write_word(kStackOffset + expected_pc_stack_addr,
                                registers.pc + 2));
    EXPECT_CALL(mmu, write_byte(kStackOffset + expected_p_stack_addr,
                                registers.p | B_FLAG));

    step_execution(7);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, php) {
    stage_instruction(PHP);
    registers.sp = 0x0A;
    registers.p = 0xBB;

    expected.sp = 0x09;
    expected.p = registers.p;
    ++expected.pc;

    const uint8_t p_size = 1;
    const uint8_t expected_stack_addr = registers.sp - p_size;

    EXPECT_CALL(mmu,
                write_byte(kStackOffset + expected_stack_addr, registers.p));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, clc) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLC);
    expected.p &= ~C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sec) {
    stage_instruction(SEC);
    expected.p |= C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_shifts) {
    stage_instruction(LSR_A);
    registers.a = 0b01001000;
    expected.a = 0b00100100;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_z_flag) {
    stage_instruction(LSR_A);
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_flag) {
    stage_instruction(LSR_A);
    registers.a = 0b00000011;
    expected.a = 0b00000001;
    expected.p = C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_and_z_flags) {
    stage_instruction(LSR_A);
    registers.a = 0b00000001;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_clears_c_and_z_flags) {
    stage_instruction(LSR_A);
    registers.a = 0b00000010;
    registers.p = Z_FLAG | C_FLAG | N_FLAG;
    expected.a = 0b00000001;
    expected.p = N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pha) {
    stage_instruction(PHA);
    registers.sp = 0x05;
    registers.a = 0x84;

    const uint8_t a_size = 1;
    const uint8_t expected_stack_addr = registers.sp - a_size;

    EXPECT_CALL(mmu,
                write_byte(kStackOffset + expected_stack_addr, registers.a));

    step_execution(3);

    ++expected.pc;
    expected.sp = 0x04;
    expected.a = registers.a;

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jmp) {
    stage_instruction(JMP);
    expected.pc = 0x1234;

    ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x1234));

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cli) {
    stage_instruction(CLI);
    registers.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sei) {
    stage_instruction(SEI);
    expected.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_sets_y) {
    stage_instruction(LDY_I);
    expected.y = 42;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_sets_n_flag) {
    stage_instruction(LDY_I);
    expected.y = 128;
    expected.p |= N_FLAG;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_clears_n_flag) {
    stage_instruction(LDY_I);
    registers.p |= N_FLAG;
    expected.y = 127;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_sets_z_flag) {
    stage_instruction(LDY_I);
    expected.y = 0;
    expected.p |= Z_FLAG;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_clears_z_flag) {
    stage_instruction(LDY_I);
    registers.p |= Z_FLAG;
    expected.y = 1;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, clv) {
    stage_instruction(CLV);
    registers.p |= V_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cld) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLD);
    expected.p &= ~D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, nop) {
    stage_instruction(NOP);

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inx_increments) {
    stage_instruction(INX);
    expected.x += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inx_sets_n_flag) {
    stage_instruction(INX);
    registers.x = 127;
    expected.x = 128;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inx_clears_n_flag) {
    stage_instruction(INX);
    registers.x = 255;
    registers.p |= N_FLAG;
    expected.x = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inx_sets_z_flag) {
    stage_instruction(INX);
    registers.x = 255;
    expected.x = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inx_clears_z_flag) {
    stage_instruction(INX);
    registers.x = 0;
    registers.p |= Z_FLAG;
    expected.x = 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sed) {
    stage_instruction(SED);
    expected.p |= D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

} // namespace
