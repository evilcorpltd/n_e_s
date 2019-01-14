#include "core/cpu_factory.h"

#include "hexprinter.h"
#include "mock_mmu.h"

#include <gtest/gtest.h>
#include <ostream>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::NiceMock;
using testing::Return;

namespace n_e_s::core {

static bool operator==(const ICpu::Registers &a, const ICpu::Registers &b) {
    return a.pc == b.pc && a.sp == b.sp && a.a == b.a && a.x == b.x &&
           a.y == b.y && a.p == b.p;
}

static void PrintTo(const ICpu::Registers &r, std::ostream *os) {
    *os << "PC: " << hex_out_s(r.pc);
    *os << " SP: " << hex_out_s(r.sp);
    *os << " A: " << hex_out_s(r.a);
    *os << " X: " << hex_out_s(r.x);
    *os << " Y: " << hex_out_s(r.y);
    *os << " P: ";
    *os << (r.p & N_FLAG ? "N" : "-");
    *os << (r.p & V_FLAG ? "V" : "-");
    *os << "-";
    *os << (r.p & B_FLAG ? "B" : "-");
    *os << (r.p & D_FLAG ? "D" : "-");
    *os << (r.p & I_FLAG ? "I" : "-");
    *os << (r.p & Z_FLAG ? "Z" : "-");
    *os << (r.p & C_FLAG ? "C" : "-");
    *os << std::endl;
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
    BPL = 0x10,
    CLC = 0x18,
    BMI = 0x30,
    SEC = 0x38,
    LSR_ACC = 0x4A,
    PHA = 0x48,
    JMP = 0x4C,
    BVC = 0x50,
    CLI = 0x58,
    BVS = 0x70,
    SEI = 0x78,
    TXA = 0x8A,
    STY_ABS = 0x8C,
    STA_ABS = 0x8D,
    STX_ABS = 0x8E,
    STY_ZERO = 0x84,
    STA_ZERO = 0x85,
    STX_ZERO = 0x86,
    BCC = 0x90,
    STY_ZEROX = 0x94,
    STA_ZEROX = 0x95,
    STX_ZEROY = 0x96,
    TYA = 0x98,
    TXS = 0x9A,
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
    BNE = 0xD0,
    CLD = 0xD8,
    NOP = 0xEA,
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
        ON_CALL(mmu, read_byte(static_cast<uint8_t>(0x44 + 0xED)))
                .WillByDefault(Return(0x42));

        step_execution(4);
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
    ++expected.pc;

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

TEST_F(CpuTest, clc) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLC);
    expected.p &= ~C_FLAG;

    step_execution(2);
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

TEST_F(CpuTest, cli) {
    stage_instruction(CLI);
    registers.p |= I_FLAG;

    step_execution(2);
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
    EXPECT_CALL(mmu, write_byte(static_cast<uint8_t>(0x44 + 0xED), 0x07));

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
    EXPECT_CALL(mmu, write_byte(static_cast<uint8_t>(0x44 + 0xED), 0x07));

    step_execution(4);

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
    EXPECT_CALL(mmu, write_byte(static_cast<uint8_t>(0x44 + 0xED), 0x07));

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

} // namespace
