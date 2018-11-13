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
    CpuTest() : mmu(), cpu(&mmu), registers(cpu.registers) {
    }

    void stage_instruction(uint8_t instruction) {
        ON_CALL(mmu, read_byte(cpu.registers.pc))
                .WillByDefault(Return(instruction));
    }

    MockMmu mmu;
    Cpu cpu;
    Registers registers;
};

TEST_F(CpuTest, flag_register) {
    EXPECT_EQ(0, registers.p);
}

TEST_F(CpuTest, clc) {
    cpu.registers.p |= C_FLAG | N_FLAG;

    stage_instruction(Cpu::CLC);
    cpu.execute();
    EXPECT_EQ(registers.pc + 1, cpu.registers.pc);
    EXPECT_EQ(registers.p | C_FLAG | N_FLAG, cpu.registers.p);

    cpu.execute();
    EXPECT_EQ(registers.p | N_FLAG, cpu.registers.p);
    EXPECT_EQ(registers.pc + 1, cpu.registers.pc);
}

TEST_F(CpuTest, sec) {
    stage_instruction(Cpu::SEC);
    cpu.execute();
    EXPECT_EQ(registers.pc + 1, cpu.registers.pc);
    EXPECT_EQ(registers.p, cpu.registers.p);

    cpu.execute();
    EXPECT_EQ(registers.p | C_FLAG, cpu.registers.p);
    EXPECT_EQ(registers.pc + 1, cpu.registers.pc);
}

TEST_F(CpuTest, nop) {
    stage_instruction(Cpu::NOP);
    cpu.execute();
    registers.pc += 1;
    EXPECT_EQ(registers, cpu.registers);

    cpu.execute();
    EXPECT_EQ(registers, cpu.registers);
}

TEST_F(CpuTest, register_instructions) {
    cpu.sed();
    EXPECT_EQ(cpu.registers.p, D_FLAG);

    cpu.cld();
    EXPECT_EQ(cpu.registers.p, 0);
}

TEST_F(CpuTest, inx) {
    EXPECT_EQ(cpu.registers.x, 0);
    cpu.inx();
    EXPECT_EQ(cpu.registers.x, 1);
    EXPECT_EQ(cpu.registers.p, 0);
    cpu.inx();
    cpu.inx();
    EXPECT_EQ(cpu.registers.x, 3);
    EXPECT_EQ(cpu.registers.p, 0);
    cpu.registers.x = 126;
    cpu.inx();
    EXPECT_EQ(cpu.registers.x, 127);
    EXPECT_EQ(cpu.registers.p, 0);
    cpu.inx();
    EXPECT_EQ(cpu.registers.p, N_FLAG);
    cpu.registers.x = 255;
    cpu.inx();
    EXPECT_EQ(cpu.registers.x, 0);
    EXPECT_EQ(cpu.registers.p, Z_FLAG);
}

TEST_F(CpuTest, lsr) {
    cpu.lsr_a();
    assert(cpu.registers.p == Z_FLAG);
}

}
