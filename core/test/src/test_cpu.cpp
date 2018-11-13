// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

#include "mock_mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::_;
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

    Registers registers;
    MockMmu mmu;
    Cpu cpu;

    Registers expected;
};

TEST_F(CpuTest, flag_register) {
    EXPECT_EQ(0, registers.p);
}

TEST_F(CpuTest, clc) {
    registers.p |= C_FLAG | N_FLAG;

    stage_instruction(Cpu::CLC);
    cpu.execute();
    EXPECT_EQ(expected.pc + 1, registers.pc);
    EXPECT_EQ(expected.p | C_FLAG | N_FLAG, registers.p);

    cpu.execute();
    EXPECT_EQ(expected.p | N_FLAG, registers.p);
    EXPECT_EQ(expected.pc + 1, registers.pc);
}

TEST_F(CpuTest, sec) {
    stage_instruction(Cpu::SEC);
    cpu.execute();
    EXPECT_EQ(expected.pc + 1, registers.pc);
    EXPECT_EQ(expected.p, registers.p);

    cpu.execute();
    EXPECT_EQ(expected.p | C_FLAG, registers.p);
    EXPECT_EQ(expected.pc + 1, registers.pc);
}

TEST_F(CpuTest, nop) {
    stage_instruction(Cpu::NOP);
    cpu.execute();
    expected.pc += 1;
    EXPECT_EQ(expected, registers);

    cpu.execute();
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, register_instructions) {
    cpu.sed();
    EXPECT_EQ(registers.p, D_FLAG);

    cpu.cld();
    EXPECT_EQ(registers.p, 0);
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

TEST_F(CpuTest, lsr) {
    cpu.lsr_a();
    assert(registers.p == Z_FLAG);
}

}
