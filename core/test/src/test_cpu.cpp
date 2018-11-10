// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"

#include <gtest/gtest.h>

class CpuTest : public ::testing::Test {
public:
    CpuTest() : cpu(), registers(cpu.registers) {
    }

    CPU cpu;
    Registers registers;
};

TEST_F(CpuTest, flag_register) {
    EXPECT_EQ(0, registers.p);
}

TEST_F(CpuTest, nop) {
    const uint8_t cycles = cpu.nop();

    EXPECT_EQ(registers.sp, cpu.registers.sp);
    EXPECT_EQ(registers.a, cpu.registers.a);
    EXPECT_EQ(registers.x, cpu.registers.x);
    EXPECT_EQ(registers.y, cpu.registers.y);
    EXPECT_EQ(registers.p, cpu.registers.p);
    EXPECT_EQ(2, cycles);
}

TEST_F(CpuTest, register_instructions) {
    cpu.sec();
    cpu.sed();
    EXPECT_EQ(cpu.registers.p, C_FLAG | D_FLAG);

    cpu.cld();
    EXPECT_EQ(cpu.registers.p, C_FLAG);

    cpu.clc();
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
