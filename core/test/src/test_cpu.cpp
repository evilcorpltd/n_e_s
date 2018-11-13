// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

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

class CpuTest : public ::testing::Test {
public:
    CpuTest() : registers(), mmu(), cpu(&registers, &mmu), expected() {
    }

    void stage_instruction(uint8_t instruction) {
        ON_CALL(mmu, read_byte(registers.pc))
                .WillByDefault(Return(instruction));
    }

    void step_execution(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            cpu.execute();
        }
    }

    Registers registers;
    NiceMock<MockMmu> mmu;
    Cpu cpu;

    Registers expected;
};

TEST_F(CpuTest, clc) {
    expected.p = registers.p = 0xFF;

    stage_instruction(Cpu::CLC);
    expected.pc += 1;
    expected.p &= ~C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sec) {
    stage_instruction(Cpu::SEC);
    expected.pc += 1;
    expected.p |= C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cld) {
    expected.p = registers.p = 0xFF;

    stage_instruction(Cpu::CLD);
    expected.pc += 1;
    expected.p &= ~D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, nop) {
    stage_instruction(Cpu::NOP);
    expected.pc += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sed) {
    stage_instruction(Cpu::SED);
    expected.pc += 1;
    expected.p |= D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inx) {
    EXPECT_EQ(registers.x, 0);
    cpu.inx();
    EXPECT_EQ(registers.x, 1);
    EXPECT_EQ(registers.p, 0);
    cpu.inx();
    cpu.inx();
    EXPECT_EQ(registers.x, 3);
    EXPECT_EQ(registers.p, 0);
    registers.x = 126;
    cpu.inx();
    EXPECT_EQ(registers.x, 127);
    EXPECT_EQ(registers.p, 0);
    cpu.inx();
    EXPECT_EQ(registers.p, N_FLAG);
    registers.x = 255;
    cpu.inx();
    EXPECT_EQ(registers.x, 0);
    EXPECT_EQ(registers.p, Z_FLAG);
}

TEST_F(CpuTest, lsr_a_shifts_correctly) {
    stage_instruction(Cpu::LSR_A);
    registers.a = 0b01001000;
    expected.a = 0b00100100;
    expected.pc += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_z_flag) {
    stage_instruction(Cpu::LSR_A);
    expected.pc += 1;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_flag) {
    stage_instruction(Cpu::LSR_A);
    registers.a = 0b00000011;
    expected.a = 0b00000001;
    expected.pc += 1;
    expected.p = C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_and_z_flags) {
    stage_instruction(Cpu::LSR_A);
    registers.a = 0b00000001;
    expected.a = 0b00000000;
    expected.pc += 1;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_clears_c_and_z_flags) {
    stage_instruction(Cpu::LSR_A);
    registers.a = 0b00000010;
    registers.p = Z_FLAG | C_FLAG | N_FLAG;
    expected.a = 0b00000001;
    expected.pc += 1;
    expected.p = N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

}
