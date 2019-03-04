#include "core/cpu_factory.h"

#include "icpu_helpers.h"
#include "mock_mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::NiceMock;
using testing::Return;

namespace {

constexpr uint8_t u16_to_u8(uint16_t u16) {
    return u16 % 0x100;
}

const uint16_t kStackOffset = 0x0100;
const uint16_t kResetAddress = 0xFFFC;
const uint16_t kBrkAddress = 0xFFFE;

// Tests and opcodes should be written without looking at the cpu
// implementation. Look at a data sheet and don't cheat!
enum Opcode : uint8_t {
    BRK = 0x00,
    PHP = 0x08,
    BPL = 0x10,
    BIT_ZERO = 0x24,
    BIT_ABS = 0x2C,
    CLC = 0x18,
    JSR = 0x20,
    BMI = 0x30,
    SEC = 0x38,
    LSR_ACC = 0x4A,
    PHA = 0x48,
    JMP = 0x4C,
    BVC = 0x50,
    CLI = 0x58,
    RTS = 0x60,
    ADC_ZERO = 0x65,
    ADC_IMM = 0x69,
    ADC_ABS = 0x6D,
    BVS = 0x70,
    SEI = 0x78,
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
    TSX = 0xBA,
    CPY_IMM = 0xC0,
    CPY_ZERO = 0xC4,
    INY = 0xC8,
    DEX = 0xCA,
    CPY_ABS = 0xCC,
    BNE = 0xD0,
    CLD = 0xD8,
    CPX_IMM = 0xE0,
    CPX_ZERO = 0xE4,
    NOP = 0xEA,
    CPX_ABS = 0xEC,
    BEQ = 0xF0,
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

    void branch_test(uint8_t branch_flag,
            int8_t offset,
            uint8_t expected_cycles) {
        expected.p = registers.p = branch_flag;
        expected.pc = registers.pc + 2 + offset;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(offset));

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

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_sets_n_flag(uint8_t *reg) {
        *reg = 128;
        expected.p |= N_FLAG;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_clears_n_flag(uint8_t *reg) {
        registers.p |= N_FLAG;
        *reg = 127;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_sets_z_flag(uint8_t *reg) {
        *reg = 0;
        expected.p |= Z_FLAG;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_immediate_clears_z_flag(uint8_t *reg) {
        registers.p |= Z_FLAG;
        *reg = 1;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(*reg));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void load_absolute_sets_reg(uint8_t *reg) {
        *reg = 0x42;
        expected.pc += 2;

        ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
        ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(*reg));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void load_absolute_sets_n_flag(uint8_t *reg) {
        *reg = 128;
        expected.p |= N_FLAG;
        expected.pc += 2;

        ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
        ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(*reg));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void load_absolute_clears_n_flag(uint8_t *reg) {
        registers.p |= N_FLAG;
        *reg = 127;
        expected.pc += 2;

        ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
        ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(*reg));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void load_absolute_sets_z_flag(uint8_t *reg) {
        *reg = 0;
        expected.p |= Z_FLAG;
        expected.pc += 2;

        ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
        ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(*reg));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void load_absolute_clears_z_flag(uint8_t *reg) {
        registers.p |= Z_FLAG;
        *reg = 1;
        expected.pc += 2;

        ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
        ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(*reg));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void load_zeropage_sets_reg(uint8_t instruction, uint8_t *target_reg) {
        registers.pc = expected.pc = 0x4321;

        stage_instruction(instruction);

        *target_reg = 0x42;
        expected.pc += 1;

        ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
        ON_CALL(mmu, read_byte(0x44)).WillByDefault(Return(0x42));

        step_execution(3);
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

        ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
        ON_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED)))
                .WillByDefault(Return(0x42));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void compare_immediate_sets_n_clears_z_c(uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0b10001111;
        expected.p |= N_FLAG;
        registers.p |= Z_FLAG | C_FLAG;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1))
                .WillByDefault(Return(0b00001000));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void compare_immediate_sets_c_clears_n_z(uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 0x02;
        expected.p |= C_FLAG;
        registers.p |= Z_FLAG | N_FLAG;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(0xFA));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void compare_immediate_sets_z_c_clears_n(uint8_t *reg,
            uint8_t *expected_reg) {
        *expected_reg = *reg = 42;
        expected.p |= Z_FLAG | C_FLAG;
        registers.p |= N_FLAG;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(42));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void compare_immediate_sets_n_c(uint8_t *reg, uint8_t *expected_reg) {
        *expected_reg = *reg = 0;
        expected.p |= N_FLAG | C_FLAG;
        registers.p |= Z_FLAG;
        ++expected.pc;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(127));

        step_execution(2);
        EXPECT_EQ(expected, registers);
    }

    void compare_abs_sets_n_c(uint8_t *reg, uint8_t *expected_reg) {
        *expected_reg = *reg = 0;
        expected.p |= N_FLAG | C_FLAG;
        registers.p |= Z_FLAG;
        expected.pc += 2;

        ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
        ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(127));

        step_execution(4);
        EXPECT_EQ(expected, registers);
    }

    void compare_zeropage_sets_n_c(uint8_t *reg, uint8_t *expected_reg) {
        *expected_reg = *reg = 0;
        expected.p |= N_FLAG | C_FLAG;
        registers.p |= Z_FLAG;
        expected.pc += 1;

        ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(0x44));
        ON_CALL(mmu, read_byte(0x44)).WillByDefault(Return(127));

        step_execution(3);
        EXPECT_EQ(expected, registers);
    }

    ICpu::Registers registers;
    NiceMock<MockMmu> mmu;
    std::unique_ptr<ICpu> cpu;

    ICpu::Registers expected;
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

    const uint8_t expected_pc_stack_addr = expected.sp - 1;
    const uint8_t expected_p_stack_addr = expected_pc_stack_addr - 1;

    expected.sp -= 2 + 1; // 1 word and 1 byte

    ON_CALL(mmu, read_word(kBrkAddress)).WillByDefault(Return(0xDEAD));

    // First the return address is pushed and then the registers.
    EXPECT_CALL(mmu,
            write_word(
                    kStackOffset + expected_pc_stack_addr, registers.pc + 2));
    EXPECT_CALL(mmu,
            write_byte(kStackOffset + expected_p_stack_addr,
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

    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, registers.p));

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

TEST_F(CpuTest, bit_zero_sets_zero) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x00;

    stage_instruction(BIT_ZERO);

    expected.pc += 1;
    expected.p = Z_FLAG;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    ON_CALL(mmu, read_byte(0x44)).WillByDefault(Return(0x12));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, bit_zero_sets_negative) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;

    stage_instruction(BIT_ZERO);

    expected.pc += 1;
    expected.p = N_FLAG;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    ON_CALL(mmu, read_byte(0x44)).WillByDefault(Return(0xA3));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, bit_zero_sets_overflow) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;

    stage_instruction(BIT_ZERO);

    expected.pc += 1;
    expected.p = V_FLAG;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    ON_CALL(mmu, read_byte(0x44)).WillByDefault(Return(0x53));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, bit_abs_sets_negative_and_overflow) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x02;
    registers.p = Z_FLAG;

    stage_instruction(BIT_ABS);

    expected.pc += 2;
    expected.p = V_FLAG | N_FLAG;

    ON_CALL(mmu, read_word(0x4322)).WillByDefault(Return(0x5678));
    ON_CALL(mmu, read_byte(0x5678)).WillByDefault(Return(0xC3));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, clc) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLC);
    expected.p &= ~C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jsr) {
    stage_instruction(JSR);
    expected.pc = 0xDEAD;

    const uint8_t expected_pc_stack_addr = expected.sp - 1;
    expected.sp -= 2; // 1 word

    EXPECT_CALL(mmu, read_word(registers.pc + 1)).WillOnce(Return(0xDEAD));

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
    registers.p = Z_FLAG | C_FLAG | N_FLAG;
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

    ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x1234));

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

TEST_F(CpuTest, adc_imm_no_carry_or_overflow) {
    stage_instruction(ADC_IMM);

    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(0x10));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_imm_carry_but_no_overflow) {
    stage_instruction(ADC_IMM);

    registers.a = 0xD0;
    registers.p = V_FLAG;
    expected.p = C_FLAG;
    expected.a = 0x20;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(0x50));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_imm_no_carry_but_overflow) {
    stage_instruction(ADC_IMM);

    registers.a = 0x50;
    expected.p = V_FLAG | N_FLAG;
    expected.a = 0xA0;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(0x50));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_imm_carry_and_overflow) {
    stage_instruction(ADC_IMM);

    registers.a = 0xD0;
    expected.p = V_FLAG | C_FLAG;
    expected.a = 0x60;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(0x90));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_abs_no_carry_or_overflow) {
    stage_instruction(ADC_ABS);

    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    expected.pc += 2;

    ON_CALL(mmu, read_word(registers.pc + 1)).WillByDefault(Return(0x4567));
    ON_CALL(mmu, read_byte(0x4567)).WillByDefault(Return(0x10));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, adc_zero_no_carry_or_overflow) {
    registers.pc = expected.pc = 0x4321;

    stage_instruction(ADC_ZERO);

    registers.a = 0x50;
    registers.p = V_FLAG;
    expected.a = 0x60;
    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x45));
    ON_CALL(mmu, read_byte(0x45)).WillByDefault(Return(0x10));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cli) {
    stage_instruction(CLI);
    registers.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, rts) {
    stage_instruction(RTS);
    registers.sp = 0x0A;
    expected.sp = 0x0C;
    expected.pc = 0xDEAD + 1;

    EXPECT_CALL(mmu, read_word(kStackOffset + 0x0B)).WillOnce(Return(0xDEAD));

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
TEST_F(CpuTest, ldx_abs_sets_reg) {
    stage_instruction(LDX_ABS);
    load_absolute_sets_reg(&expected.x);
}
TEST_F(CpuTest, ldx_abs_sets_n_flag) {
    stage_instruction(LDX_ABS);
    load_absolute_sets_n_flag(&expected.x);
}
TEST_F(CpuTest, ldx_abs_clears_n_flag) {
    stage_instruction(LDX_ABS);
    load_absolute_clears_n_flag(&expected.x);
}
TEST_F(CpuTest, ldx_abs_sets_z_flag) {
    stage_instruction(LDX_ABS);
    load_absolute_sets_z_flag(&expected.x);
}
TEST_F(CpuTest, ldx_abs_clears_z_flag) {
    stage_instruction(LDX_ABS);
    load_absolute_clears_z_flag(&expected.x);
}

// LDY Absolute mode
TEST_F(CpuTest, ldy_abs_sets_reg) {
    stage_instruction(LDY_ABS);
    load_absolute_sets_reg(&expected.y);
}
TEST_F(CpuTest, ldy_abs_sets_n_flag) {
    stage_instruction(LDY_ABS);
    load_absolute_sets_n_flag(&expected.y);
}
TEST_F(CpuTest, ldy_abs_clears_n_flag) {
    stage_instruction(LDY_ABS);
    load_absolute_clears_n_flag(&expected.y);
}
TEST_F(CpuTest, ldy_abs_sets_z_flag) {
    stage_instruction(LDY_ABS);
    load_absolute_sets_z_flag(&expected.y);
}
TEST_F(CpuTest, ldy_abs_clears_z_flag) {
    stage_instruction(LDY_ABS);
    load_absolute_clears_z_flag(&expected.y);
}

// LDA Absolute mode
TEST_F(CpuTest, lda_abs_sets_reg) {
    stage_instruction(LDA_ABS);
    load_absolute_sets_reg(&expected.a);
}
TEST_F(CpuTest, lda_abs_sets_n_flag) {
    stage_instruction(LDA_ABS);
    load_absolute_sets_n_flag(&expected.a);
}
TEST_F(CpuTest, lda_abs_clears_n_flag) {
    stage_instruction(LDA_ABS);
    load_absolute_clears_n_flag(&expected.a);
}
TEST_F(CpuTest, lda_abs_sets_z_flag) {
    stage_instruction(LDA_ABS);
    load_absolute_sets_z_flag(&expected.a);
}
TEST_F(CpuTest, lda_abs_clears_z_flag) {
    stage_instruction(LDA_ABS);
    load_absolute_clears_z_flag(&expected.a);
}

// LD Zeropage
TEST_F(CpuTest, lda_zeropage_sets_reg) {
    load_zeropage_sets_reg(LDA_ZERO, &expected.a);
}
TEST_F(CpuTest, ldx_zeropage_sets_reg) {
    load_zeropage_sets_reg(LDX_ZERO, &expected.x);
}
TEST_F(CpuTest, ldy_zeropage_sets_reg) {
    load_zeropage_sets_reg(LDY_ZERO, &expected.y);
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
    expected.p &= ~D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// CPX Immediate mode
TEST_F(CpuTest, cpx_imm_sets_n_and_clears_z_c) {
    stage_instruction(CPX_IMM);
    compare_immediate_sets_n_clears_z_c(&registers.x, &expected.x);
}
TEST_F(CpuTest, cpx_imm_sets_c_and_clears_n_z) {
    stage_instruction(CPX_IMM);
    compare_immediate_sets_c_clears_n_z(&registers.x, &expected.x);
}
TEST_F(CpuTest, cpx_imm_sets_z_c_clears_n) {
    stage_instruction(CPX_IMM);
    compare_immediate_sets_z_c_clears_n(&registers.x, &expected.x);
}
TEST_F(CpuTest, cpx_imm_sets_nc) {
    stage_instruction(CPX_IMM);
    compare_immediate_sets_n_c(&registers.x, &expected.x);
}

// CPY Immediate mode
TEST_F(CpuTest, cpy_imm_sets_n_and_clears_z_c) {
    stage_instruction(CPY_IMM);
    compare_immediate_sets_n_clears_z_c(&registers.y, &expected.y);
}
TEST_F(CpuTest, cpy_imm_sets_c_and_clears_n_z) {
    stage_instruction(CPY_IMM);
    compare_immediate_sets_c_clears_n_z(&registers.y, &expected.y);
}
TEST_F(CpuTest, cpy_imm_sets_z_c_clears_n) {
    stage_instruction(CPY_IMM);
    compare_immediate_sets_z_c_clears_n(&registers.y, &expected.y);
}
TEST_F(CpuTest, cpy_imm_sets_nc) {
    stage_instruction(CPY_IMM);
    compare_immediate_sets_n_c(&registers.y, &expected.y);
}

// CPX, CPY Absolute mode
TEST_F(CpuTest, cpx_abs_sets_nc) {
    stage_instruction(CPX_ABS);
    compare_abs_sets_n_c(&registers.x, &expected.x);
}
TEST_F(CpuTest, cpy_abs_sets_nc) {
    stage_instruction(CPY_ABS);
    compare_abs_sets_n_c(&registers.y, &expected.y);
}

// CPX, CPY Zeropage mode
TEST_F(CpuTest, cpx_zeropage_sets_nc) {
    stage_instruction(CPX_ZERO);
    compare_zeropage_sets_n_c(&registers.x, &expected.x);
}
TEST_F(CpuTest, cpy_zeropage_sets_nc) {
    stage_instruction(CPY_ZERO);
    compare_zeropage_sets_n_c(&registers.y, &expected.y);
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

TEST_F(CpuTest, sta_abs) {
    registers.pc = expected.pc = 0x1121;
    registers.a = expected.a = 0x45;

    stage_instruction(STA_ABS);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x1122)).WillOnce(Return(0x0987));
    EXPECT_CALL(mmu, write_byte(0x0987, 0x45));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_zero) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_ZERO);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    EXPECT_CALL(mmu, write_byte(0x44, 0x07));

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_zero_x_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0x07;
    registers.x = expected.x = 0xED;

    stage_instruction(STA_ZEROX);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x07));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, stx_abs) {
    registers.pc = expected.pc = 0x158;
    registers.x = expected.x = 0x71;

    stage_instruction(STX_ABS);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x0159)).WillOnce(Return(0x1987));
    EXPECT_CALL(mmu, write_byte(0x1987, 0x71));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, stx_zero) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x07;

    stage_instruction(STX_ZERO);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    EXPECT_CALL(mmu, write_byte(0x44, 0x07));

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, stx_zero_y_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x07;
    registers.y = expected.y = 0xED;

    stage_instruction(STX_ZEROY);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
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

    ON_CALL(mmu, read_word(0x4322)).WillByDefault(Return(0x1234));
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

    ON_CALL(mmu, read_word(0x4322)).WillByDefault(Return(0x1234));
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

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0xAB));
    ON_CALL(mmu, read_word(u16_to_u8(0xAB + 0xED)))
            .WillByDefault(Return(0x1234));
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

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0xFF));
    ON_CALL(mmu, read_byte(0xFF)).WillByDefault(Return(0x34));
    ON_CALL(mmu, read_byte(0x00)).WillByDefault(Return(0x12));
    EXPECT_CALL(mmu, write_byte(0x1234, 0x07));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indirect_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INDINX);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x42));
    ON_CALL(mmu, read_word(0x42)).WillByDefault(Return(0x1234));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0xED, 0x07));

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

TEST_F(CpuTest, sty_abs) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x07;

    stage_instruction(STY_ABS);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_word(0x4322)).WillOnce(Return(0x4444));
    EXPECT_CALL(mmu, write_byte(0x4444, 0x07));

    step_execution(4);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sty_zero) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x07;

    stage_instruction(STY_ZERO);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
    EXPECT_CALL(mmu, write_byte(0x44, 0x07));

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sty_zero_x_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x07;
    registers.x = expected.x = 0xED;

    stage_instruction(STY_ZEROX);

    expected.pc += 1;

    ON_CALL(mmu, read_byte(0x4322)).WillByDefault(Return(0x44));
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

} // namespace
