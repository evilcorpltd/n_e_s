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
    return a.pc == b.pc
            && a.sp == b.sp
            && a.a == b.a
            && a.x == b.x
            && a.y == b.y
            && a.p == b.p;
}

}

namespace {

// Tests and opcodes should be written without looking at the cpu
// implementation. Look at a data sheet and don't cheat!
enum Opcode : uint8_t {
    CLC = 0x18,
    SEC = 0x38,
    LSR_A = 0x4A,
    CLI = 0x58,
    SEI = 0x78,
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
              expected() {
    }

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

}
