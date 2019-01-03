#include "core/cpu_factory.h"

#include "hexprinter.h"
#include "mock_mmu.h"

#include <gtest/gtest.h>
#include <bitset>
#include <ostream>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::NiceMock;
using testing::Return;

namespace n_e_s::core {

static bool operator==(const Registers &a, const Registers &b) {
    return a.pc == b.pc && a.sp == b.sp && a.a == b.a && a.x == b.x &&
           a.y == b.y && a.p == b.p;
}

static void PrintTo(const Registers &r, std::ostream *os) {
    *os << "PC: " << hex_out_s(r.pc);
    *os << " SP: " << hex_out_s(r.sp);
    *os << " A: " << hex_out_s(r.a);
    *os << " X: " << hex_out_s(r.x);
    *os << " Y: " << hex_out_s(r.y);
    *os << " P: 0b" << std::bitset<8>(r.p) << std::endl;
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
    TXS = 0x9A,
    LDY_IMM = 0xA0,
    BCS = 0xB0,
    CLV = 0xB8,
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

TEST_F(CpuTest, ldy_i_sets_y) {
    stage_instruction(LDY_IMM);
    expected.y = 42;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_sets_n_flag) {
    stage_instruction(LDY_IMM);
    expected.y = 128;
    expected.p |= N_FLAG;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_clears_n_flag) {
    stage_instruction(LDY_IMM);
    registers.p |= N_FLAG;
    expected.y = 127;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_sets_z_flag) {
    stage_instruction(LDY_IMM);
    expected.y = 0;
    expected.p |= Z_FLAG;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ldy_i_clears_z_flag) {
    stage_instruction(LDY_IMM);
    registers.p |= Z_FLAG;
    expected.y = 1;
    ++expected.pc;

    ON_CALL(mmu, read_byte(registers.pc + 1)).WillByDefault(Return(expected.y));

    step_execution(2);
    EXPECT_EQ(expected, registers);
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

} // namespace
