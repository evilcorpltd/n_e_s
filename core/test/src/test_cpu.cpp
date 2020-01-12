#include "core/cpu_factory.h"

#include "fake_ppu.h"
#include "icpu_helpers.h"
#include "mock_mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::InSequence;
using testing::NiceMock;
using testing::Return;

namespace {

constexpr uint8_t u16_to_u8(uint16_t u16) {
    return static_cast<uint8_t>(u16 % static_cast<uint16_t>(0x100));
}

const uint16_t kStackOffset = 0x0100;
const uint16_t kResetAddress = 0xFFFC;
const uint16_t kBrkAddress = 0xFFFE;
const uint16_t kNmiAddress = 0xFFFA;

// Tests and opcodes should be written without looking at the cpu
// implementation. Look at a data sheet and don't cheat!
enum Opcode : uint8_t {
    BRK = 0x00,
    ORA_ZERO = 0x05,
    ASL_ZERO = 0x06,
    PHP = 0x08,
    ORA_IMM = 0x09,
    ASL_ACC = 0x0A,
    ORA_ABS = 0x0D,
    ASL_ABS = 0x0E,
    BPL = 0x10,
    ORA_ZEROX = 0x15,
    CLC = 0x18,
    ORA_ABSY = 0x19,
    ORA_ABSX = 0x1D,
    JSR = 0x20,
    BIT_ZERO = 0x24,
    PLP = 0x28,
    AND_IMM = 0x29,
    ROL_ACC = 0x2A,
    BIT_ABS = 0x2C,
    AND_ABS = 0x2D,
    BMI = 0x30,
    SEC = 0x38,
    AND_ABSY = 0x39,
    AND_ABSX = 0x3D,
    RTI = 0x40,
    EOR_INXIND = 0x41,
    EOR_ZERO = 0x45,
    LSR_ACC = 0x4A,
    PHA = 0x48,
    EOR_IMM = 0x49,
    JMP = 0x4C,
    EOR_ABS = 0x4D,
    BVC = 0x50,
    EOR_INDINX = 0x51,
    EOR_ZEROX = 0x55,
    CLI = 0x58,
    EOR_ABSY = 0x59,
    EOR_ABSX = 0x5D,
    RTS = 0x60,
    ADC_INDX = 0x61,
    ADC_ZERO = 0x65,
    PLA = 0x68,
    ADC_IMM = 0x69,
    ROR_ACC = 0x6A,
    ADC_ABS = 0x6D,
    BVS = 0x70,
    ADC_INDY = 0x71,
    ADC_ZEROX = 0x75,
    SEI = 0x78,
    ADC_ABSY = 0x79,
    ADC_ABSX = 0x7D,
    STA_INXIND = 0x81,
    TXA = 0x8A,
    STY_ABS = 0x8C,
    STA_ABS = 0x8D,
    STX_ABS = 0x8E,
    STY_ZERO = 0x84,
    STA_ZERO = 0x85,
    STX_ZERO = 0x86,
    DEY = 0x88,
    BCC = 0x90,
    STA_INDINX = 0x91,
    STY_ZEROX = 0x94,
    STA_ZEROX = 0x95,
    STX_ZEROY = 0x96,
    TYA = 0x98,
    STA_ABSY = 0x99,
    TXS = 0x9A,
    STA_ABSX = 0x9D,
    LDY_IMM = 0xA0,
    LDX_IMM = 0xA2,
    LDY_ZERO = 0xA4,
    LDA_ZERO = 0xA5,
    LDX_ZERO = 0xA6,
    TAY = 0xA8,
    LDA_IMM = 0xA9,
    TAX = 0xAA,
    LDY_ABS = 0xAC,
    LDA_ABS = 0xAD,
    LDX_ABS = 0xAE,
    BCS = 0xB0,
    LDY_ZEROX = 0xB4,
    LDA_ZEROX = 0xB5,
    LDX_ZEROY = 0xB6,
    CLV = 0xB8,
    LDA_ABSY = 0xB9,
    TSX = 0xBA,
    LDY_ABSX = 0xBC,
    LDA_ABSX = 0xBD,
    LDX_ABSY = 0xBE,
    CPY_IMM = 0xC0,
    CMP_INXIND = 0xC1,
    CPY_ZERO = 0xC4,
    CMP_ZERO = 0xC5,
    DEC_ZERO = 0xC6,
    INY = 0xC8,
    CMP_IMM = 0xC9,
    DEX = 0xCA,
    CPY_ABS = 0xCC,
    CMP_ABS = 0xCD,
    BNE = 0xD0,
    CMP_INDINX = 0xD1,
    CMP_ZEROX = 0xD5,
    DEC_ZEROX = 0xD6,
    CLD = 0xD8,
    CMP_ABSY = 0xD9,
    CMP_ABSX = 0xDD,
    CPX_IMM = 0xE0,
    SBC_INXIND = 0xE1,
    CPX_ZERO = 0xE4,
    SBC_ZERO = 0xE5,
    INC_ZERO = 0xE6,
    INX = 0xE8,
    SBC_IMM = 0xE9,
    NOP = 0xEA,
    CPX_ABS = 0xEC,
    SBC_ABS = 0xED,
    BEQ = 0xF0,
    SBC_INDINX = 0xF1,
    SBC_ZEROX = 0xF5,
    INC_ZEROX = 0xF6,
    SED = 0xF8,
    SBC_ABSY = 0xF9,
    SBC_ABSX = 0xFD,
};

class CpuTest : public ::testing::Test {
public:
    CpuTest()
            : registers(),
              mmu(),
              ppu(),
              cpu{CpuFactory::create_mos6502(&registers, &mmu, &ppu)},
              expected() {
        registers.sp = expected.sp = 0xFF;
    }

    void stage_instruction(uint8_t instruction) {
        expected.pc += 1;
        EXPECT_CALL(mmu, read_byte(registers.pc)).WillOnce(Return(instruction));
    }

    void step_execution(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            cpu->execute();
        }
    }

    void branch_test(uint8_t branch_flag,
            int8_t offset,
            uint8_t expected_cycles) {
        expected.p = registers.p = branch_flag;
        expected.pc = registers.pc + static_cast<uint8_t>(2) + offset;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(offset));

        step_execution(expected_cycles);
        EXPECT_EQ(expected, registers);
    }

    // This is a bit silly, but we need both registers.from and expected.from.
    void move_test(uint8_t *expected_dest,
            uint8_t *expected_from,
            uint8_t *registers_from) {
        *expected_from = *registers_from = 10;
        *expected_dest = 10;

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void move_test_sets_n(uint8_t *expected_dest,
            uint8_t *expected_from,
            uint8_t *registers_from) {
        *expected_from = *registers_from = 128;
        *expected_dest = 128;
        expected.p |= N_FLAG;

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void move_test_clears_n(uint8_t *expected_dest,
            uint8_t *expected_from,
            uint8_t *registers_from) {
        *expected_from = *registers_from = 5;
        *expected_dest = 5;
        registers.p |= N_FLAG;

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void move_test_sets_z(uint8_t *expected_dest,
            uint8_t *expected_from,
            uint8_t *registers_from) {
        *expected_from = *registers_from = 0;
        *expected_dest = 0;
        expected.p |= Z_FLAG;

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void move_test_clears_z(uint8_t *expected_dest,
            uint8_t *expected_from,
            uint8_t *registers_from) {
        *expected_from = *registers_from = 5;
        *expected_dest = 5;
        registers.p |= Z_FLAG;

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_sets_reg(uint8_t *reg) {
        *reg = 42;
        ++expected.pc;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_sets_n_flag(uint8_t *reg) {
        *reg = 128;
        expected.p |= N_FLAG;
        ++expected.pc;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_clears_n_flag(uint8_t *reg) {
        registers.p |= N_FLAG;
        *reg = 127;
        ++expected.pc;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_sets_z_flag(uint8_t *reg) {
        *reg = 0;
        expected.p |= Z_FLAG;
        ++expected.pc;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_clears_z_flag(uint8_t *reg) {
        registers.p |= Z_FLAG;
        *reg = 1;
        ++expected.pc;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_zeropage_reg_sets_reg(uint8_t instruction,
            uint8_t *target_reg,
            uint8_t *index_reg,
            uint8_t *expected_index_reg) {
        registers.pc = expected.pc = 0x4321;
        *index_reg = *expected_index_reg = 0xED;

        stage_instruction(instruction);

        *target_reg = 0x42;
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
        EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED)))
                .WillOnce(Return(0x42));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void compare_abs_sets_n_c(uint8_t *reg, uint8_t *expected_reg) {
        *expected_reg = *reg = 0;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        expected.pc += 2;

        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x4567));
        EXPECT_CALL(mmu, read_byte(0x4567)).WillOnce(Return(127));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void compare_abs_indexed_sets_cz_without_pagecrossing(uint8_t *index_reg,
            uint8_t *index_reg_expected) {
        registers.a = expected.a = 0x07;
        *index_reg = *index_reg_expected = 0x10;
        registers.p |= N_FLAG;
        expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);

        expected.pc += 2;

        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x5678));
        EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0x07));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void compare_abs_indexed_sets_cz_with_pagecrossing(uint8_t *index_reg,
            uint8_t *index_reg_expected) {
        registers.a = expected.a = 0x07;
        *index_reg = *index_reg_expected = 0xAB;
        registers.p |= N_FLAG;
        expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);

        expected.pc += 2;

        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x5678));
        EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB - 0x0100))
                .WillOnce(Return(0xDEAD));
        EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB)).WillOnce(Return(0x07));

        step_execution(5);
        EXPECT_EQ(expected, registers);
    }

    void absolute_indexed_load_sets_register(uint8_t instruction,
            uint8_t *target_reg,
            uint8_t *index_reg,
            uint8_t *expected_index_reg) {
        registers.pc = expected.pc = 0;
        *index_reg = *expected_index_reg = 0x42;

        stage_instruction(instruction);
        expected.pc += 2;

        *target_reg = 0x37;
        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x0100));
        EXPECT_CALL(mmu, read_byte(0x0100 + *index_reg))
                .WillOnce(Return(*target_reg));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void absolute_indexed_load_sets_reg_crossing_page(uint8_t instruction,
            uint8_t *target_reg,
            uint8_t *index_reg,
            uint8_t *expected_index_reg) {
        registers.pc = expected.pc = 0;
        *index_reg = *expected_index_reg = 0x01;

        stage_instruction(instruction);
        expected.pc += 2;

        *target_reg = 0x37;
        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x01FF));

        EXPECT_CALL(mmu, read_byte(0x00FF + *index_reg));
        EXPECT_CALL(mmu, read_byte(0x01FF + *index_reg))
                .WillOnce(Return(*target_reg));
        step_execution(5);

        EXPECT_EQ(expected, registers);
    }

    void absolute_indexed_load_sets_z(uint8_t instruction,
            uint8_t *target_reg,
            uint8_t *index_reg,
            uint8_t *expected_index_reg) {
        expected.p |= Z_FLAG;
        registers.p |= N_FLAG;
        registers.pc = expected.pc = 0;
        *index_reg = *expected_index_reg = 0x42;

        stage_instruction(instruction);
        expected.pc += 2;

        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x0100));
        EXPECT_CALL(mmu, read_byte(0x0100 + 0x42)).WillOnce(Return(0));

        *target_reg = 0;

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void absolute_indexed_load_sets_n(uint8_t instruction,
            uint8_t *target_reg,
            uint8_t *index_reg,
            uint8_t *expected_index_reg) {
        expected.p |= N_FLAG;
        registers.p |= Z_FLAG;
        registers.pc = expected.pc = 0;
        *index_reg = *expected_index_reg = 0x42;

        stage_instruction(instruction);
        expected.pc += 2;

        EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x0100));
        EXPECT_CALL(mmu, read_byte(0x0100 + 0x42)).WillOnce(Return(230));

        *target_reg = 230;

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    CpuRegisters registers;
    NiceMock<MockMmu> mmu;
    FakePpu ppu;
    std::unique_ptr<IMos6502> cpu;

    CpuRegisters expected;

    InSequence in_sequence{};
};

class CpuImmediateTest : public CpuTest {
public:
    void run_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 1;

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(memory_content));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void compare_sets_n_clears_z_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0b10001111;
        expected.p |= N_FLAG;
        registers.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
        memory_content = 0b00001000;

        run_instruction(instruction);
    }

    void compare_sets_c_clears_n_z(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0x02;
        expected.p |= C_FLAG;
        registers.p |= static_cast<uint8_t>(Z_FLAG | N_FLAG);
        memory_content = 0xFA;

        run_instruction(instruction);
    }

    void compare_sets_z_c_clears_n(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 42;
        expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);
        registers.p |= N_FLAG;
        memory_content = 42;

        run_instruction(instruction);
    }

    void compare_sets_n_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        memory_content = 127;

        run_instruction(instruction);
    }

    uint16_t start_pc{0x4321};
    uint8_t memory_content{0x42};
};

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
        *expected_reg = *reg = 0;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        memory_content = 127;

        run_read_instruction(instruction);
    }

    uint16_t start_pc{0x4321};
    uint8_t memory_content{0x42};
    uint8_t effective_address{0x44};
};

class CpuAbsoluteTest : public CpuTest {
public:
    void run_read_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const auto lower_address = static_cast<uint8_t>(
                effective_address & static_cast<uint16_t>(0x00FFu));
        const auto upper_address = static_cast<uint8_t>(
                static_cast<uint16_t>(effective_address & 0xFF00u) >> 8u);

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(lower_address));
        EXPECT_CALL(mmu, read_byte(start_pc + 2))
                .WillOnce(Return(upper_address));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_write_instruction(uint8_t instruction) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const auto lower_address = static_cast<uint8_t>(
                effective_address & static_cast<uint16_t>(0x00FFu));
        const auto upper_address =
                static_cast<uint8_t>((effective_address & 0xFF00u) >> 8u);

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(lower_address));
        EXPECT_CALL(mmu, read_byte(start_pc + 2))
                .WillOnce(Return(upper_address));
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void run_readwrite_instruction(uint8_t instruction,
            uint8_t new_memory_content) {
        registers.pc = expected.pc = start_pc;
        stage_instruction(instruction);
        expected.pc += 2;

        const auto lower_address = static_cast<uint8_t>(
                effective_address & static_cast<uint16_t>(0x00FFu));
        const auto upper_address =
                static_cast<uint8_t>((effective_address & 0xFF00u) >> 8u);

        EXPECT_CALL(mmu, read_byte(start_pc + 1))
                .WillOnce(Return(lower_address));
        EXPECT_CALL(mmu, read_byte(start_pc + 2))
                .WillOnce(Return(upper_address));
        EXPECT_CALL(mmu, read_byte(effective_address))
                .WillOnce(Return(memory_content));
        // Dummy write
        EXPECT_CALL(mmu, write_byte(effective_address, memory_content));
        EXPECT_CALL(mmu, write_byte(effective_address, new_memory_content));

        step_execution(6);
        EXPECT_EQ(expected, registers);
    }

    void compare_abs_sets_n_c(uint8_t instruction,
            uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0;
        expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
        registers.p |= Z_FLAG;
        memory_content = 127;
        run_read_instruction(instruction);
    }

    void load_absolute_sets_reg(uint8_t instruction, uint8_t *reg) {
        *reg = 0x42;
        memory_content = *reg;
        run_read_instruction(instruction);
    }

    void load_absolute_sets_n_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 128;
        memory_content = *reg;
        expected.p |= N_FLAG;
        run_read_instruction(instruction);
    }

    void load_absolute_clears_n_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 127;
        memory_content = *reg;
        registers.p |= N_FLAG;
        run_read_instruction(instruction);
    }

    void load_absolute_sets_z_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 0;
        memory_content = *reg;
        expected.p |= Z_FLAG;
        run_read_instruction(instruction);
    }

    void load_absolute_clears_z_flag(uint8_t instruction, uint8_t *reg) {
        *reg = 1;
        memory_content = *reg;
        registers.p |= Z_FLAG;
        run_read_instruction(instruction);
    }

    uint16_t start_pc{0x1121};
    uint8_t memory_content{0x42};
    uint16_t effective_address{0x4567};
};

TEST_F(CpuTest, reset) {
    expected.pc = 0xDEAD;
    EXPECT_CALL(mmu, read_word(kResetAddress)).WillOnce(Return(0xDEAD));

    cpu->reset();

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, reset_clears_pipeline) {
    stage_instruction(SEC);
    EXPECT_CALL(mmu, read_word(kResetAddress)).WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, read_byte(0xDEAD)).WillOnce(Return(0x00));
    expected.pc = 0xDEAD + 1;

    cpu->execute(); // Stage things for execution.
    cpu->reset();
    cpu->execute(); // Should read an opcode from 0xDEAD and not execute what's
                    // been staged.

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, state_is_initialized) {
    const auto state = cpu->state();

    EXPECT_FALSE(state.current_opcode.has_value());
    EXPECT_EQ(0u, state.start_pc);
    EXPECT_EQ(0u, state.start_cycle);
    EXPECT_EQ(0u, state.cycle);
}

TEST_F(CpuTest, state_opcode_returns_instruction) {
    registers.pc = 0x1234;
    stage_instruction(PHP);
    step_execution(1);

    const CpuState state = cpu->state();
    const auto opcode = state.current_opcode;

    EXPECT_EQ(0u, state.start_cycle);
    EXPECT_EQ(0x1234, state.start_pc);
    EXPECT_EQ(1u, state.cycle);

    EXPECT_TRUE(opcode.has_value());
    EXPECT_EQ(Instruction::PhpImplied, opcode->instruction);
    EXPECT_EQ(Family::PHP, opcode->family);
    EXPECT_EQ(AddressMode::Implied, opcode->address_mode);

    // Only cycle should change
    step_execution(1);
    const CpuState next_state = cpu->state();
    EXPECT_EQ(0u, next_state.start_cycle);
    EXPECT_EQ(0x1234, next_state.start_pc);
    EXPECT_EQ(2u, next_state.cycle);
}

TEST_F(CpuTest, unsupported_instruction) {
    stage_instruction(0xFF);

    EXPECT_THROW(step_execution(1), std::logic_error);
}

TEST_F(CpuTest, nmi) {
    registers.pc = 0x1234;
    stage_instruction(LDA_ZERO);
    cpu->set_nmi(true);

    expected.sp -= 2 + 1; // 1 word and 1 byte

    // Dummy reads
    // Read from 0x1234 is done when parsing the LDA instruction, so PC is at
    // 0x1235 when nmi is run.
    EXPECT_CALL(mmu, read_byte(0x1235));
    EXPECT_CALL(mmu, read_byte(0x1236));

    expected.pc = 0x5678; // nmi vector

    // First the return address is pushed and then the registers.
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, 0x12));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 1, 0x37));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 2, registers.p));

    // Read nmi vector 0x5678
    EXPECT_CALL(mmu, read_byte(kNmiAddress)).WillOnce(Return(0x78));
    EXPECT_CALL(mmu, read_byte(kNmiAddress + 1)).WillOnce(Return(0x56));

    // 1 instruction to decode LDA, 7 for nmi
    step_execution(1 + 7);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, brk) {
    registers.pc = 0x1234;
    stage_instruction(BRK);
    expected.pc = 0xDEAD;

    expected.sp -= 2 + 1; // 1 word and 1 byte

    // Dummy read
    EXPECT_CALL(mmu, read_byte(0x1235));

    // First the return address is pushed and then the registers.
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, 0x12));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 1, 0x36));
    EXPECT_CALL(mmu,
            write_byte(kStackOffset + registers.sp - 2, registers.p | B_FLAG));

    EXPECT_CALL(mmu, read_byte(kBrkAddress)).WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kBrkAddress + 1)).WillOnce(Return(0xDE));

    step_execution(7);

    EXPECT_EQ(expected, registers);
}

// ASL
TEST_F(CpuTest, asl_acc_ignores_carry) {
    stage_instruction(ASL_ACC);
    registers.p = C_FLAG;
    registers.a = 0b00110011;
    expected.a = 0b01100110;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, asl_acc_set_c_and_z_flags_clears_n) {
    stage_instruction(ASL_ACC);
    registers.a = 0b10000000;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuZeropageTest, asl_zero_clears_c_z_sets_n) {
    registers.p = C_FLAG | Z_FLAG;
    expected.p = N_FLAG;
    memory_content = 0b01010101;

    run_readwrite_instruction(ASL_ZERO, 0b10101010);
}
TEST_F(CpuAbsoluteTest, asl_abs_sets_z_c) {
    registers.p = N_FLAG;
    expected.p = C_FLAG | Z_FLAG;
    memory_content = 0b10000000;

    run_readwrite_instruction(ASL_ABS, 0b00000000);
}

TEST_F(CpuTest, php_sets_b_flag) {
    stage_instruction(PHP);
    registers.sp = 0x0A;
    registers.p = N_FLAG | Z_FLAG;

    expected.sp = 0x09;
    expected.p = registers.p;

    EXPECT_CALL(
            mmu, write_byte(kStackOffset + registers.sp, registers.p | B_FLAG));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bpl_branch_not_taken) {
    stage_instruction(BPL);
    expected.p = registers.p = N_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bpl_branch_taken) {
    registers.pc = 0xD321;
    stage_instruction(BPL);

    branch_test(0, 0x79, 3);
}

TEST_F(CpuTest, bpl_crossing_page_boundary) {
    registers.pc = 0xD390;
    stage_instruction(BPL);

    branch_test(0, 0x79, 4);
}

TEST_F(CpuTest, bpl_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BPL);

    branch_test(0, -128 + 5, 3);
}

TEST_F(CpuImmediateTest, and_imm) {
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    expected.a = 0b00001010;
    memory_content = 0b00001111;

    run_instruction(AND_IMM);
}

TEST_F(CpuAbsoluteTest, and_abs_sets_zero_clears_neg) {
    registers.a = 0b10101010;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b01010101;

    run_read_instruction(AND_ABS);
}

TEST_F(CpuAbsoluteTest, and_abs_sets_neg_clears_zero) {
    registers.a = 0b11000011;
    registers.p = Z_FLAG;
    expected.a = 0b11000001;
    expected.p = N_FLAG;
    memory_content = 0b11110001;

    run_read_instruction(AND_ABS);
}

TEST_F(CpuTest, and_absx_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    registers.x = expected.x = 0x10;

    stage_instruction(AND_ABSX);

    expected.pc += 2;
    expected.a = 0b00001010;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0b00001111));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, and_absx_with_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    registers.x = expected.x = 0xAB;

    stage_instruction(AND_ABSX);

    expected.pc += 2;
    expected.a = 0b00001010;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB)).WillOnce(Return(0b00001111));

    step_execution(5);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, and_absy_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;
    registers.y = expected.y = 0x10;

    stage_instruction(AND_ABSY);

    expected.pc += 2;
    expected.a = 0b00001010;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0b00001111));

    step_execution(4);
    EXPECT_EQ(expected, registers);
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
TEST_F(CpuAbsoluteTest, bit_abs_sets_negative_and_overflow) {
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;
    expected.p = V_FLAG | N_FLAG;
    memory_content = 0xC3;

    run_read_instruction(BIT_ABS);
}

TEST_F(CpuTest, plp_clears_b_and_sets_bit_5) {
    stage_instruction(PLP);
    registers.sp = 0x0A;
    registers.p = 0xBB;

    expected.sp = registers.sp + static_cast<uint8_t>(1u);
    expected.p = static_cast<uint8_t>(FLAG_5 | C_FLAG) | N_FLAG;

    EXPECT_CALL(mmu, read_byte(kStackOffset + expected.sp))
            .WillOnce(Return(static_cast<uint8_t>(B_FLAG | C_FLAG) | N_FLAG));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, clc) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLC);
    expected.p &= static_cast<uint8_t>(~C_FLAG);

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jsr) {
    stage_instruction(JSR);
    expected.pc = 0xDEAD;

    const uint8_t expected_pc_stack_addr =
            expected.sp - static_cast<uint8_t>(1);
    expected.sp -= 2; // 1 word

    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2)).WillOnce(Return(0xDE));

    EXPECT_CALL(mmu,
            write_word(
                    kStackOffset + expected_pc_stack_addr, registers.pc + 2));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bmi_branch_not_taken) {
    stage_instruction(BMI);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bmi_branch_taken) {
    registers.pc = 0xD321;
    stage_instruction(BMI);

    branch_test(N_FLAG, 0x79, 3);
}

TEST_F(CpuTest, bmi_crossing_page_boundary) {
    registers.pc = 0xD390;
    stage_instruction(BMI);

    branch_test(N_FLAG, 0x79, 4);
}

TEST_F(CpuTest, bmi_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BMI);

    branch_test(N_FLAG, -128 + 5, 3);
}

TEST_F(CpuTest, sec) {
    stage_instruction(SEC);
    expected.p |= C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_shifts) {
    stage_instruction(LSR_ACC);
    registers.a = 0b01001000;
    expected.a = 0b00100100;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_z_flag) {
    stage_instruction(LSR_ACC);
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_flag) {
    stage_instruction(LSR_ACC);
    registers.a = 0b00000011;
    expected.a = 0b00000001;
    expected.p = C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_and_z_flags) {
    stage_instruction(LSR_ACC);
    registers.a = 0b00000001;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_clears_c_z_n_flags) {
    stage_instruction(LSR_ACC);
    registers.a = 0b00000010;
    registers.p = static_cast<uint8_t>(Z_FLAG | C_FLAG) | N_FLAG;
    expected.a = 0b00000001;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pha) {
    stage_instruction(PHA);
    registers.sp = 0x05;
    registers.a = 0x84;

    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, registers.a));

    expected.sp = 0x04;
    expected.a = registers.a;

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jmp) {
    stage_instruction(JMP);
    expected.pc = 0x1234;

    EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0x1234));

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bvc_branch_not_taken) {
    stage_instruction(BVC);
    expected.p = registers.p = V_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bvc_branch_taken) {
    registers.pc = 0xA350;
    stage_instruction(BVC);

    branch_test(0, 0x10, 3);
}

TEST_F(CpuTest, bvc_crossing_page_boundary) {
    registers.pc = 0xD390;
    stage_instruction(BVC);

    branch_test(0, 0x70, 4);
}

TEST_F(CpuTest, bvc_negative_operand) {
    registers.pc = 0xA350;
    stage_instruction(BVC);

    branch_test(0, -128 + 10, 3);
}

// ADC
TEST_F(CpuImmediateTest, adc_imm_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    memory_content = 0x10;

    run_instruction(ADC_IMM);
}

TEST_F(CpuImmediateTest, adc_imm_carry_but_no_overflow) {
    registers.a = 0xD0;
    registers.p = V_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x20;
    memory_content = 0x50;

    run_instruction(ADC_IMM);
}

TEST_F(CpuImmediateTest, adc_imm_no_carry_but_overflow) {
    registers.a = 0x50;
    expected.p = V_FLAG | N_FLAG;
    expected.a = 0xA0;
    memory_content = 0x50;

    run_instruction(ADC_IMM);
}

TEST_F(CpuImmediateTest, adc_imm_carry_and_overflow) {
    registers.a = 0xD0;
    expected.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0x90;

    run_instruction(ADC_IMM);
}

TEST_F(CpuAbsoluteTest, adc_abs_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    memory_content = 0x10;

    run_read_instruction(ADC_ABS);
}

TEST_F(CpuZeropageTest, adc_zero_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    memory_content = 0x10;

    run_read_instruction(ADC_ZERO);
}

TEST_F(CpuTest, adc_absx_no_carry_or_overflow_no_pagecrossing) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x10;

    stage_instruction(ADC_ABSX);

    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x71;
    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0x21));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_absy_no_carry_or_overflow_with_pagecrossing) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xAB;

    stage_instruction(ADC_ABSY);

    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x71;
    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB)).WillOnce(Return(0x21));

    step_execution(5);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_zero_x) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0x50;
    registers.x = expected.x = 0xED;
    registers.p = V_FLAG;

    stage_instruction(ADC_ZEROX);

    expected.a = 0x57;
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x07));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_indexed_indirect) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = 0x07;
    expected.a = 0x07 + 0x12;

    stage_instruction(ADC_INDX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_word(u16_to_u8(0xAB + 0xED)))
            .WillOnce(Return(0x1234));
    EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(0x12));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_indirect_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x0D;
    registers.a = 0x07;
    expected.a = 0x07 + 0x12;

    stage_instruction(ADC_INDY);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x0D)).WillOnce(Return(0x12));
    step_execution(5);

    EXPECT_EQ(expected, registers);
}

// SBC
TEST_F(CpuImmediateTest, sbc_imm_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbc_imm_carry_but_no_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x20;
    memory_content = 0x30;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbc_imm_no_carry_but_overflow) {
    registers.a = 0x50;
    registers.p = C_FLAG;
    expected.p = V_FLAG | N_FLAG;
    expected.a = 0xA0;
    memory_content = 0xB0;

    run_instruction(SBC_IMM);
}

TEST_F(CpuImmediateTest, sbc_imm_carry_and_overflow) {
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0x70;

    run_instruction(SBC_IMM);
}

TEST_F(CpuAbsoluteTest, sbc_abs_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0;

    run_read_instruction(SBC_ABS);
}

TEST_F(CpuZeropageTest, sbc_zero_no_carry_or_overflow) {
    registers.a = 0x50;
    registers.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    memory_content = 0xF0;

    run_read_instruction(SBC_ZERO);
}

TEST_F(CpuTest, sbc_absx_no_carry_or_overflow_no_pagecrossing) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x10;

    stage_instruction(SBC_ABSX);

    registers.a = 0xD0;
    registers.p = V_FLAG | C_FLAG;
    expected.p = N_FLAG;
    expected.a = 0xE0;
    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0xF0));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sbc_absy_no_carry_or_overflow_with_pagecrossing) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xAB;

    stage_instruction(SBC_ABSY);

    registers.a = 0xD0;
    registers.p = V_FLAG | C_FLAG;
    expected.p = N_FLAG;
    expected.a = 0xE0;
    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB)).WillOnce(Return(0xF0));

    step_execution(5);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sbc_zero_x) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0x50;
    registers.x = expected.x = 0xED;
    registers.p = V_FLAG | C_FLAG;

    stage_instruction(SBC_ZEROX);

    expected.p = C_FLAG;
    expected.a = 0x50 - 0x07;
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x07));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sbc_indexed_indirect) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;

    stage_instruction(SBC_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_word(u16_to_u8(0xAB + 0xED)))
            .WillOnce(Return(0x1234));
    EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(0xF0));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sbc_indirect_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x0D;
    registers.a = 0xD0;
    registers.p = C_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;

    stage_instruction(SBC_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x0D)).WillOnce(Return(0xF0));
    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pla_sets_z_clears_n) {
    stage_instruction(PLA);
    registers.sp = 0x0A;
    registers.a = 0xBB;
    registers.p = N_FLAG;

    expected.sp = registers.sp + static_cast<uint8_t>(1u);
    expected.a = 0x00;
    expected.p = Z_FLAG;

    EXPECT_CALL(mmu, read_byte(kStackOffset + expected.sp))
            .WillOnce(Return(0x00));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pla_sets_n_clears_z) {
    stage_instruction(PLA);
    registers.sp = 0x0A;
    registers.a = 0xBB;
    registers.p = Z_FLAG;

    expected.sp = registers.sp + static_cast<uint8_t>(1u);
    expected.a = 0x92;
    expected.p = N_FLAG;

    EXPECT_CALL(mmu, read_byte(kStackOffset + expected.sp))
            .WillOnce(Return(0x92));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cli) {
    stage_instruction(CLI);
    registers.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, rts) {
    registers.pc = 0x1234;
    stage_instruction(RTS);
    registers.sp = 0x0A;
    expected.sp = 0x0C;
    expected.pc = 0xDEAD + 1;

    // Dummy read
    EXPECT_CALL(mmu, read_byte(0x1235));

    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 1))
            .WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 2))
            .WillOnce(Return(0xDE));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, rti) {
    registers.pc = 0x1234;
    stage_instruction(RTI);
    registers.sp = 0x0A;
    expected.p = static_cast<uint8_t>(Z_FLAG | C_FLAG) | FLAG_5;
    expected.sp = registers.sp + static_cast<uint8_t>(3);
    expected.pc = 0xDEAD;

    // Dummy read
    EXPECT_CALL(mmu, read_byte(0x1235));

    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 1))
            .WillOnce(Return(static_cast<uint8_t>(Z_FLAG | C_FLAG) | B_FLAG));
    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 2))
            .WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 3))
            .WillOnce(Return(0xDE));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bvs_branch_not_taken) {
    stage_instruction(BVS);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bvs_branch_taken) {
    registers.pc = 0xC300;
    stage_instruction(BVS);

    branch_test(V_FLAG, 127, 3);
}

TEST_F(CpuTest, bvs_crossing_page_boundary) {
    registers.pc = 0xC3FD;
    stage_instruction(BVS);

    branch_test(V_FLAG, 3, 4);
}

TEST_F(CpuTest, bvs_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BVS);

    branch_test(V_FLAG, -128 + 70, 3);
}

TEST_F(CpuTest, sei) {
    stage_instruction(SEI);
    expected.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bcc_branch_not_taken) {
    stage_instruction(BCC);
    expected.p = registers.p = C_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bcc_branch_taken) {
    registers.pc = 0x0010;
    stage_instruction(BCC);

    branch_test(0, 0x10, 3);
}

TEST_F(CpuTest, bcc_crossing_page_negative) {
    registers.pc = 0x0010;
    stage_instruction(BCC);

    branch_test(0, -70, 4);
}

// LDX Immediate mode
TEST_F(CpuTest, ldx_i_sets_reg) {
    stage_instruction(LDX_IMM);
    load_immediate_sets_reg(&expected.x);
}
TEST_F(CpuTest, ldx_i_sets_n_flag) {
    stage_instruction(LDX_IMM);
    load_immediate_sets_n_flag(&expected.x);
}
TEST_F(CpuTest, ldx_i_clears_n_flag) {
    stage_instruction(LDX_IMM);
    load_immediate_clears_n_flag(&expected.x);
}
TEST_F(CpuTest, ldx_i_sets_z_flag) {
    stage_instruction(LDX_IMM);
    load_immediate_sets_z_flag(&expected.x);
}
TEST_F(CpuTest, ldx_i_clears_z_flag) {
    stage_instruction(LDX_IMM);
    load_immediate_clears_z_flag(&expected.x);
}

// LDY Immediate mode
TEST_F(CpuTest, ldy_i_sets_reg) {
    stage_instruction(LDY_IMM);
    load_immediate_sets_reg(&expected.y);
}
TEST_F(CpuTest, ldy_i_sets_n_flag) {
    stage_instruction(LDY_IMM);
    load_immediate_sets_n_flag(&expected.y);
}
TEST_F(CpuTest, ldy_i_clears_n_flag) {
    stage_instruction(LDY_IMM);
    load_immediate_clears_n_flag(&expected.y);
}
TEST_F(CpuTest, ldy_i_sets_z_flag) {
    stage_instruction(LDY_IMM);
    load_immediate_sets_z_flag(&expected.y);
}
TEST_F(CpuTest, ldy_i_clears_z_flag) {
    stage_instruction(LDY_IMM);
    load_immediate_clears_z_flag(&expected.y);
}

// LDA Immediate mode
TEST_F(CpuTest, lda_i_sets_reg) {
    stage_instruction(LDA_IMM);
    load_immediate_sets_reg(&expected.a);
}
TEST_F(CpuTest, lda_i_sets_n_flag) {
    stage_instruction(LDA_IMM);
    load_immediate_sets_n_flag(&expected.a);
}
TEST_F(CpuTest, lda_i_clears_n_flag) {
    stage_instruction(LDA_IMM);
    load_immediate_clears_n_flag(&expected.a);
}
TEST_F(CpuTest, lda_i_sets_z_flag) {
    stage_instruction(LDA_IMM);
    load_immediate_sets_z_flag(&expected.a);
}
TEST_F(CpuTest, lda_i_clears_z_flag) {
    stage_instruction(LDA_IMM);
    load_immediate_clears_z_flag(&expected.a);
}

// LDX Absolute mode
TEST_F(CpuAbsoluteTest, ldx_abs_sets_reg) {
    load_absolute_sets_reg(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_sets_n_flag) {
    load_absolute_sets_n_flag(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_clears_n_flag) {
    load_absolute_clears_n_flag(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_sets_z_flag) {
    load_absolute_sets_z_flag(LDX_ABS, &expected.x);
}
TEST_F(CpuAbsoluteTest, ldx_abs_clears_z_flag) {
    load_absolute_clears_z_flag(LDX_ABS, &expected.x);
}

// LDY Absolute mode
TEST_F(CpuAbsoluteTest, ldy_abs_sets_reg) {
    load_absolute_sets_reg(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_sets_n_flag) {
    load_absolute_sets_n_flag(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_clears_n_flag) {
    load_absolute_clears_n_flag(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_sets_z_flag) {
    load_absolute_sets_z_flag(LDY_ABS, &expected.y);
}
TEST_F(CpuAbsoluteTest, ldy_abs_clears_z_flag) {
    load_absolute_clears_z_flag(LDY_ABS, &expected.y);
}

// LDA Absolute mode
TEST_F(CpuAbsoluteTest, lda_abs_sets_reg) {
    load_absolute_sets_reg(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_sets_n_flag) {
    load_absolute_sets_n_flag(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_clears_n_flag) {
    load_absolute_clears_n_flag(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_sets_z_flag) {
    load_absolute_sets_z_flag(LDA_ABS, &expected.a);
}
TEST_F(CpuAbsoluteTest, lda_abs_clears_z_flag) {
    load_absolute_clears_z_flag(LDA_ABS, &expected.a);
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

// LD Zeropage X
TEST_F(CpuTest, lda_zeropagex_sets_reg) {
    load_zeropage_reg_sets_reg(
            LDA_ZEROX, &expected.a, &registers.x, &expected.x);
}
TEST_F(CpuTest, ldy_zeropagex_sets_reg) {
    load_zeropage_reg_sets_reg(
            LDY_ZEROX, &expected.y, &registers.x, &expected.x);
}

// LD Zeropage Y
TEST_F(CpuTest, ldx_zeropagey_sets_reg) {
    load_zeropage_reg_sets_reg(
            LDX_ZEROY, &expected.x, &registers.y, &expected.y);
}

// LD absolute indexed
// LDA_ABSY
TEST_F(CpuTest, lda_abs_y_sets_reg) {
    absolute_indexed_load_sets_register(
            LDA_ABSY, &expected.a, &registers.y, &expected.y);
}
TEST_F(CpuTest, lda_abs_y_sets_reg_crossing_page) {
    absolute_indexed_load_sets_reg_crossing_page(
            LDA_ABSY, &expected.a, &registers.y, &expected.y);
}
TEST_F(CpuTest, lda_abs_y_sets_z_flag) {
    absolute_indexed_load_sets_z(
            LDA_ABSY, &expected.a, &registers.y, &expected.y);
}
TEST_F(CpuTest, lda_abs_y_sets_n) {
    absolute_indexed_load_sets_register(
            LDA_ABSY, &expected.a, &registers.y, &expected.y);
}
// LDY_ABSX
TEST_F(CpuTest, ldy_abs_x_sets_reg) {
    absolute_indexed_load_sets_register(
            LDY_ABSX, &expected.y, &registers.x, &expected.x);
}
TEST_F(CpuTest, ldy_abs_x_sets_reg_crossing_page) {
    absolute_indexed_load_sets_reg_crossing_page(
            LDY_ABSX, &expected.y, &registers.x, &expected.x);
}
TEST_F(CpuTest, ldy_abs_x_sets_z_flag) {
    absolute_indexed_load_sets_z(
            LDY_ABSX, &expected.y, &registers.x, &expected.x);
}
TEST_F(CpuTest, ldy_abs_x_sets_n) {
    absolute_indexed_load_sets_register(
            LDY_ABSX, &expected.y, &registers.x, &expected.x);
}
// LDA_ABSX
TEST_F(CpuTest, lda_abs_x_sets_reg) {
    absolute_indexed_load_sets_register(
            LDA_ABSX, &expected.a, &registers.x, &expected.x);
}
TEST_F(CpuTest, lda_abs_x_sets_reg_crossing_page) {
    absolute_indexed_load_sets_reg_crossing_page(
            LDA_ABSX, &expected.a, &registers.x, &expected.x);
}
TEST_F(CpuTest, lda_abs_x_sets_z_flag) {
    absolute_indexed_load_sets_z(
            LDA_ABSX, &expected.a, &registers.x, &expected.x);
}
TEST_F(CpuTest, lda_abs_x_sets_n) {
    absolute_indexed_load_sets_register(
            LDA_ABSX, &expected.a, &registers.x, &expected.x);
}
// LDX_ABSY
TEST_F(CpuTest, ldx_abs_y_sets_reg) {
    absolute_indexed_load_sets_register(
            LDX_ABSY, &expected.x, &registers.y, &expected.y);
}
TEST_F(CpuTest, ldx_abs_y_sets_reg_crossing_page) {
    absolute_indexed_load_sets_reg_crossing_page(
            LDX_ABSY, &expected.x, &registers.y, &expected.y);
}
TEST_F(CpuTest, ldx_abs_y_sets_z_flag) {
    absolute_indexed_load_sets_z(
            LDX_ABSY, &expected.x, &registers.y, &expected.y);
}
TEST_F(CpuTest, ldx_abs_y_sets_n) {
    absolute_indexed_load_sets_register(
            LDX_ABSY, &expected.x, &registers.y, &expected.y);
}

// BCS
TEST_F(CpuTest, bcs_branch_not_taken) {
    stage_instruction(BCS);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bcs_branch_taken) {
    registers.pc = 0xC3F0;
    stage_instruction(BCS);

    branch_test(C_FLAG, 5, 3);
}

TEST_F(CpuTest, bcs_crossing_page_boundary) {
    registers.pc = 0xC3FD;
    stage_instruction(BCS);

    branch_test(C_FLAG, 3, 4);
}

TEST_F(CpuTest, bcs_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BCS);

    branch_test(C_FLAG, -128 + 127, 3);
}

TEST_F(CpuTest, clv) {
    stage_instruction(CLV);
    registers.p |= V_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bne_branch_not_taken) {
    stage_instruction(BNE);
    expected.p = registers.p = Z_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bne_branch_taken) {
    registers.pc = 0xFF7A;
    stage_instruction(BNE);

    branch_test(0, 5, 3);
}

TEST_F(CpuTest, bne_crossing_page_negative) {
    registers.pc = 0xFF7A;
    stage_instruction(BNE);

    branch_test(0, -127, 4);
}

TEST_F(CpuTest, cld) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLD);
    expected.p &= static_cast<uint8_t>(~D_FLAG);

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// CPX Immediate mode
TEST_F(CpuImmediateTest, cpx_imm_sets_n_and_clears_z_c) {
    compare_sets_n_clears_z_c(CPX_IMM, &registers.x, &expected.x);
}
TEST_F(CpuImmediateTest, cpx_imm_sets_c_and_clears_n_z) {
    compare_sets_c_clears_n_z(CPX_IMM, &registers.x, &expected.x);
}
TEST_F(CpuImmediateTest, cpx_imm_sets_z_c_clears_n) {
    compare_sets_z_c_clears_n(CPX_IMM, &registers.x, &expected.x);
}
TEST_F(CpuImmediateTest, cpx_imm_sets_nc) {
    compare_sets_n_c(CPX_IMM, &registers.x, &expected.x);
}

// CPY Immediate mode
TEST_F(CpuImmediateTest, cpy_imm_sets_n_and_clears_z_c) {
    compare_sets_n_clears_z_c(CPY_IMM, &registers.y, &expected.y);
}
TEST_F(CpuImmediateTest, cpy_imm_sets_c_and_clears_n_z) {
    compare_sets_c_clears_n_z(CPY_IMM, &registers.y, &expected.y);
}
TEST_F(CpuImmediateTest, cpy_imm_sets_z_c_clears_n) {
    compare_sets_z_c_clears_n(CPY_IMM, &registers.y, &expected.y);
}
TEST_F(CpuImmediateTest, cpy_imm_sets_nc) {
    compare_sets_n_c(CPY_IMM, &registers.y, &expected.y);
}

// CMP Immediate mode
TEST_F(CpuImmediateTest, cmp_imm_sets_n_and_clears_z_c) {
    compare_sets_n_clears_z_c(CMP_IMM, &registers.a, &expected.a);
}
TEST_F(CpuImmediateTest, cmp_imm_sets_c_and_clears_n_z) {
    compare_sets_c_clears_n_z(CMP_IMM, &registers.a, &expected.a);
}
TEST_F(CpuImmediateTest, cmp_imm_sets_z_c_clears_n) {
    compare_sets_z_c_clears_n(CMP_IMM, &registers.a, &expected.a);
}
TEST_F(CpuImmediateTest, cmp_imm_sets_nc) {
    compare_sets_n_c(CMP_IMM, &registers.a, &expected.a);
}

// CPX, CPY, CMP Absolute mode
TEST_F(CpuAbsoluteTest, cpx_abs_sets_nc) {
    compare_abs_sets_n_c(CPX_ABS, &registers.x, &expected.x);
}
TEST_F(CpuAbsoluteTest, cpy_abs_sets_nc) {
    compare_abs_sets_n_c(CPY_ABS, &registers.y, &expected.y);
}
TEST_F(CpuAbsoluteTest, cmp_abs_sets_nc) {
    compare_abs_sets_n_c(CMP_ABS, &registers.a, &expected.a);
}

// CMP Absolute indexed mode
TEST_F(CpuTest, cmp_absx_sets_nc_without_pagecrossing) {
    stage_instruction(CMP_ABSX);
    compare_abs_indexed_sets_cz_without_pagecrossing(&registers.x, &expected.x);
}

TEST_F(CpuTest, cmp_absx_sets_nc_with_pagecrossing) {
    stage_instruction(CMP_ABSX);
    compare_abs_indexed_sets_cz_with_pagecrossing(&registers.x, &expected.x);
}

TEST_F(CpuTest, cmp_absy_sets_nc_without_pagecrossing) {
    stage_instruction(CMP_ABSY);
    compare_abs_indexed_sets_cz_without_pagecrossing(&registers.y, &expected.y);
}

TEST_F(CpuTest, cmp_absy_sets_nc_with_pagecrossing) {
    stage_instruction(CMP_ABSY);
    compare_abs_indexed_sets_cz_with_pagecrossing(&registers.y, &expected.y);
}

// CMP Indirect indexed mode
TEST_F(CpuTest, cmp_indirect_indexed_with_pagecrossing_sets_nc) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xED;
    registers.a = expected.a = 0x07;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;

    stage_instruction(CMP_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED - 0x0100)).WillOnce(Return(0x00));
    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED)).WillOnce(Return(127));
    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cmp_indirect_indexed_without_pagecrossing_sets_nc) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x0D;
    registers.a = expected.a = 0x07;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;

    stage_instruction(CMP_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x0D)).WillOnce(Return(127));
    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cmp_indexed_indirect_sets_nc) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = expected.a = 0x07;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;

    stage_instruction(CMP_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_word(u16_to_u8(0xAB + 0xED)))
            .WillOnce(Return(0x1234));
    EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(127));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cmp_indexed_indirect_handles_wraparound_sets_nc) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x00;
    registers.a = expected.a = 0x07;
    expected.p |= static_cast<uint8_t>(N_FLAG | C_FLAG);
    registers.p |= Z_FLAG;

    stage_instruction(CMP_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xFF));
    EXPECT_CALL(mmu, read_byte(0xFF)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(0x00)).WillOnce(Return(0x12));
    EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(127));

    step_execution(6);

    EXPECT_EQ(expected, registers);
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

TEST_F(CpuTest, cmp_zero_x_sets_zc) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x07;
    registers.x = expected.x = 0xED;
    registers.p |= N_FLAG;
    expected.p |= static_cast<uint8_t>(Z_FLAG | C_FLAG);

    stage_instruction(CMP_ZEROX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x07));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

// NOP
TEST_F(CpuTest, nop) {
    stage_instruction(NOP);

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, beq_branch_not_taken) {
    stage_instruction(BEQ);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, beq_branch_taken) {
    registers.pc = 0xFFFB;
    stage_instruction(BEQ);

    branch_test(Z_FLAG, 5, 3);
}

TEST_F(CpuTest, beq_crossing_page_boundary) {
    registers.pc = 0xFFFB;
    stage_instruction(BEQ);

    branch_test(Z_FLAG, 125, 4);
}

TEST_F(CpuTest, beq_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BEQ);

    branch_test(Z_FLAG, -128, 3);
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

TEST_F(CpuTest, inc_zerox_increments) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;

    stage_instruction(INC_ZEROX);
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x05));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x05));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x06));

    step_execution(6);
    EXPECT_EQ(expected, registers);
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

TEST_F(CpuTest, dec_zerox_decrements) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;

    stage_instruction(DEC_ZEROX);
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x05));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x05));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x04));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

// INX
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

// INY
TEST_F(CpuTest, iny_increments) {
    stage_instruction(INY);
    expected.y += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_sets_n_flag) {
    stage_instruction(INY);
    registers.y = 127;
    expected.y = 128;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_clears_n_flag) {
    stage_instruction(INY);
    registers.y = 255;
    registers.p |= N_FLAG;
    expected.y = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_sets_z_flag) {
    stage_instruction(INY);
    registers.y = 255;
    expected.y = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_clears_z_flag) {
    stage_instruction(INY);
    registers.y = 0;
    registers.p |= Z_FLAG;
    expected.y = 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// SED
TEST_F(CpuTest, sed) {
    stage_instruction(SED);
    expected.p |= D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// STA
TEST_F(CpuAbsoluteTest, sta_abs) {
    memory_content = registers.a = expected.a = 0x45;
    run_write_instruction(STA_ABS);
}

TEST_F(CpuZeropageTest, sta_zero) {
    memory_content = registers.a = expected.a = 0x07;
    run_write_instruction(STA_ZERO);
}

TEST_F(CpuTest, sta_zero_x_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x07;
    registers.x = expected.x = 0xED;

    stage_instruction(STA_ZEROX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x07));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuAbsoluteTest, stx_abs) {
    memory_content = registers.x = expected.x = 0x71;
    run_write_instruction(STX_ABS);
}

TEST_F(CpuZeropageTest, stx_zero) {
    memory_content = registers.x = expected.x = 0x07;
    run_write_instruction(STX_ZERO);
}

TEST_F(CpuTest, stx_zero_y_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x07;
    registers.y = expected.y = 0xED;

    stage_instruction(STX_ZEROY);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x07));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_abs_x_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_ABSX);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x1234));
    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0xED, 0x07));

    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_abs_y_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_ABSY);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x1234));
    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0xED, 0x07));

    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indexed_indirect) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_word(u16_to_u8(0xAB + 0xED)))
            .WillOnce(Return(0x1234));
    EXPECT_CALL(mmu, write_byte(0x1234, 0x07));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indexed_indirect_handles_wraparound) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x00;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xFF));
    EXPECT_CALL(mmu, read_byte(0xFF)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(0x00)).WillOnce(Return(0x12));
    EXPECT_CALL(mmu, write_byte(0x1234, 0x07));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indirect_indexed_with_pagecrossing) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED - 0x0100));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0xED, 0x07));
    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indirect_indexed_without_pagecrossing) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x0D;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x0D));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0x0D, 0x07));
    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, txs) {
    registers.x = expected.x = 0xAA;

    stage_instruction(TXS);
    expected.sp = 0xAA;

    step_execution(2);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuAbsoluteTest, sty_abs) {
    memory_content = registers.y = expected.y = 0x07;
    run_write_instruction(STY_ABS);
}

TEST_F(CpuZeropageTest, sty_zero) {
    memory_content = registers.y = expected.y = 0x07;
    run_write_instruction(STY_ZERO);
}

TEST_F(CpuTest, sty_zero_x_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x07;
    registers.x = expected.x = 0xED;

    stage_instruction(STY_ZEROX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x07));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

// TSX
TEST_F(CpuTest, tsx) {
    stage_instruction(TSX);
    move_test(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuTest, tsx_sets_n_flag) {
    stage_instruction(TSX);
    move_test_sets_n(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuTest, tsx_clears_n_flag) {
    stage_instruction(TSX);
    move_test_clears_n(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuTest, tsx_sets_z_flag) {
    stage_instruction(TSX);
    move_test_sets_z(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuTest, tsx_clears_z_flag) {
    stage_instruction(TSX);
    move_test_clears_z(&expected.x, &expected.sp, &registers.sp);
}

// TYA
TEST_F(CpuTest, tya) {
    stage_instruction(TYA);
    move_test(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuTest, tya_sets_n_flag) {
    stage_instruction(TYA);
    move_test_sets_n(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuTest, tya_clears_n_flag) {
    stage_instruction(TYA);
    move_test_clears_n(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuTest, tya_sets_z_flag) {
    stage_instruction(TYA);
    move_test_sets_z(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuTest, tya_clears_z_flag) {
    stage_instruction(TYA);
    move_test_clears_z(&expected.a, &expected.y, &registers.y);
}

// TAX
TEST_F(CpuTest, tax) {
    stage_instruction(TAX);
    move_test(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuTest, tax_sets_n_flag) {
    stage_instruction(TAX);
    move_test_sets_n(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuTest, tax_clears_n_flag) {
    stage_instruction(TAX);
    move_test_clears_n(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuTest, tax_sets_z_flag) {
    stage_instruction(TAX);
    move_test_sets_z(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuTest, tax_clears_z_flag) {
    stage_instruction(TAX);
    move_test_clears_z(&expected.x, &expected.a, &registers.a);
}

// TAY
TEST_F(CpuTest, tay) {
    stage_instruction(TAY);
    move_test(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuTest, tay_sets_n_flag) {
    stage_instruction(TAY);
    move_test_sets_n(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuTest, tay_clears_n_flag) {
    stage_instruction(TAY);
    move_test_clears_n(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuTest, tay_sets_z_flag) {
    stage_instruction(TAY);
    move_test_sets_z(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuTest, tay_clears_z_flag) {
    stage_instruction(TAY);
    move_test_clears_z(&expected.y, &expected.a, &registers.a);
}

// TXA
TEST_F(CpuTest, txa) {
    stage_instruction(TXA);
    move_test(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuTest, txa_sets_n_flag) {
    stage_instruction(TXA);
    move_test_sets_n(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuTest, txa_clears_n_flag) {
    stage_instruction(TXA);
    move_test_clears_n(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuTest, txa_sets_z_flag) {
    stage_instruction(TXA);
    move_test_sets_z(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuTest, txa_clears_z_flag) {
    stage_instruction(TXA);
    move_test_clears_z(&expected.a, &expected.x, &registers.x);
}

// DEY
TEST_F(CpuTest, dey_decrements) {
    stage_instruction(DEY);
    registers.y = 3;
    expected.y = 2;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_sets_n_flag) {
    stage_instruction(DEY);
    registers.y = 0;
    expected.y = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_clears_n_flag) {
    stage_instruction(DEY);
    registers.y = 128;
    registers.p |= N_FLAG;
    expected.y = 127;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_sets_z_flag) {
    stage_instruction(DEY);
    registers.y = 1;
    expected.y = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_clears_z_flag) {
    stage_instruction(DEY);
    registers.y = 0;
    registers.p |= Z_FLAG;
    expected.y = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// DEX
TEST_F(CpuTest, dex_decrements) {
    stage_instruction(DEX);
    registers.x = 3;
    expected.x = 2;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_sets_n_flag) {
    stage_instruction(DEX);
    registers.x = 0;
    expected.x = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_clears_n_flag) {
    stage_instruction(DEX);
    registers.x = 128;
    registers.p |= N_FLAG;
    expected.x = 127;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_sets_z_flag) {
    stage_instruction(DEX);
    registers.x = 1;
    expected.x = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_clears_z_flag) {
    stage_instruction(DEX);
    registers.x = 0;
    registers.p |= Z_FLAG;
    expected.x = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// EOR
TEST_F(CpuTest, eor_imm) {
    registers.pc = expected.pc = 0x3210;
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;

    stage_instruction(EOR_IMM);

    ++expected.pc;
    expected.a = 0b00111100;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0b10010110));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuAbsoluteTest, eor_abs_set_zero_clears_neg) {
    registers.a = 0b10101010;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(EOR_ABS);
}

TEST_F(CpuAbsoluteTest, eor_abs_set_neg_clears_zero) {
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b10101010;
    expected.p = N_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(EOR_ABS);
}

TEST_F(CpuTest, eor_absx_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b11111100;
    registers.p = Z_FLAG | N_FLAG;
    registers.x = expected.x = 0x10;

    stage_instruction(EOR_ABSX);

    expected.pc += 2;
    expected.a = 0b00110011;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0b11001111));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, eor_absx_with_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b11110000;
    registers.p = N_FLAG;
    registers.x = expected.x = 0xAB;

    stage_instruction(EOR_ABSX);

    expected.pc += 2;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB)).WillOnce(Return(0b11110000));

    step_execution(5);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, eor_absy_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b00111100;
    registers.p = Z_FLAG | N_FLAG;
    registers.y = expected.y = 0x10;

    stage_instruction(EOR_ABSY);

    expected.pc += 2;
    expected.a = 0b10011001;
    expected.p = N_FLAG;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0b10100101));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, eor_indexed_indirect) {
    registers.pc = expected.pc = 0x1000;
    registers.x = expected.x = 0xED;
    registers.a = 0xE0;
    expected.a = 0x00;
    expected.p = Z_FLAG;

    stage_instruction(EOR_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x1001)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_word(u16_to_u8(0xAB + 0xED)))
            .WillOnce(Return(0x0900));
    EXPECT_CALL(mmu, read_byte(0x0900)).WillOnce(Return(0xE0));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, eor_indirect_indexed) {
    registers.pc = expected.pc = 0x9000;
    registers.y = expected.y = 0x10;
    registers.a = 0x60;
    registers.p = C_FLAG;
    expected.a = 0x90;
    expected.p = N_FLAG | C_FLAG;

    stage_instruction(EOR_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x9001)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_word(0x42)).WillOnce(Return(0x1234));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x10)).WillOnce(Return(0xF0));
    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuZeropageTest, eor_zero) {
    registers.a = 0xF0;
    expected.p = N_FLAG;
    expected.a = 0xFF;
    memory_content = 0x0F;

    run_read_instruction(EOR_ZERO);
}

TEST_F(CpuTest, eor_zero_x) {
    registers.pc = expected.pc = 0x5678;
    registers.a = 0b10000000;
    registers.x = expected.x = 0xED;
    registers.p = V_FLAG | C_FLAG;

    stage_instruction(EOR_ZEROX);

    expected.a = 0b10000001;
    expected.p = registers.p | N_FLAG;
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x5679)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x01));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

// ROL, ACC
TEST_F(CpuTest, rol_a_rotates) {
    stage_instruction(ROL_ACC);
    registers.a = 0b00110011;
    expected.a = 0b01100110;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_all_zero_no_change) {
    stage_instruction(ROL_ACC);
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_c_set_clears_c_and_z_flags) {
    stage_instruction(ROL_ACC);
    registers.a = 0b00000000;
    registers.p = C_FLAG;
    expected.a = 0b00000001;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_set_c_and_z_flags_clears_n) {
    stage_instruction(ROL_ACC);
    registers.a = 0b10000000;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_clear_neg_retain_c) {
    stage_instruction(ROL_ACC);
    registers.a = 0b10000000;
    registers.p = N_FLAG | C_FLAG;
    expected.a = 0b00000001;
    expected.p = C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// ROR, ACC
TEST_F(CpuTest, ror_a_rotates) {
    stage_instruction(ROR_ACC);
    registers.a = 0b11001100;
    expected.a = 0b01100110;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ror_a_all_zero_no_change) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ror_a_c_set_clears_c_and_sets_z_flags) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000000;
    registers.p = C_FLAG;
    expected.a = 0b10000000;
    expected.p = N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, ror_a_set_c_and_z_flags) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000001;
    registers.p = 0;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, ror_a_set_neg_retain_c) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000001;
    registers.p = C_FLAG;
    expected.a = 0b10000000;
    expected.p = C_FLAG | N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
// ORA, IMM
TEST_F(CpuTest, ora_imm) {
    registers.pc = expected.pc = 0x3210;
    registers.a = 0b10101010;
    registers.p = Z_FLAG | N_FLAG;

    stage_instruction(ORA_IMM);

    ++expected.pc;
    expected.a = 0b11111111;
    expected.p = N_FLAG;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0b01110101));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
// ORA, ABS
TEST_F(CpuAbsoluteTest, ora_abs_set_neg_clears_zero) {
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b10101010;
    expected.p = N_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(ORA_ABS);
}
// ORA, ABSY
TEST_F(CpuTest, ora_absy_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b00111100;
    registers.p = 0;
    registers.y = expected.y = 0x10;

    stage_instruction(ORA_ABSY);

    expected.pc += 2;
    expected.a = 0b11111100;
    expected.p = N_FLAG;

    ON_CALL(mmu, read_word(0x4322)).WillByDefault(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0b11110000));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}
// ORA, ABSX
TEST_F(CpuTest, ora_absx_without_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b00110000;
    registers.p = Z_FLAG | N_FLAG;
    registers.x = expected.x = 0x10;

    stage_instruction(ORA_ABSX);

    expected.pc += 2;
    expected.a = 0b00110011;

    ON_CALL(mmu, read_word(0x4322)).WillByDefault(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0x10)).WillOnce(Return(0b00100011));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, ora_absx_with_page_crossing) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b10000000;
    registers.p = N_FLAG;
    registers.x = expected.x = 0xAB;

    stage_instruction(ORA_ABSX);

    expected.pc += 2;
    expected.a = 0b10000001;
    expected.p = N_FLAG;

    ON_CALL(mmu, read_word(0x4322)).WillByDefault(Return(0x5678));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, read_byte(0x5678 + 0xAB)).WillOnce(Return(0b00000001));

    step_execution(5);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuZeropageTest, ora_zero_set_neg_clears_zero) {
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b10101010;
    expected.p = N_FLAG;
    memory_content = 0b10101010;

    run_read_instruction(ORA_ZERO);
}
TEST_F(CpuTest, ora_zero_x) {
    registers.pc = expected.pc = 0x4321;
    registers.a = 0b00110000;
    registers.p = Z_FLAG | N_FLAG;
    registers.x = expected.x = 0x44;

    stage_instruction(ORA_ZEROX);

    expected.pc += 1;
    expected.a = 0b00110011;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0x44)))
            .WillOnce(Return(0b00100011));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}
} // namespace
