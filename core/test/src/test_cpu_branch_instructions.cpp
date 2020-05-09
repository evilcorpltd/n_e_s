#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class CpuBranchTest : public CpuTest {
public:
    void branch_test(uint8_t branch_flag,
            int8_t offset,
            uint8_t expected_cycles) {
        expected.p = registers.p = branch_flag;
        expected.pc = registers.pc + static_cast<uint8_t>(2) + offset;

        EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(offset));

        step_execution(expected_cycles);
        EXPECT_EQ(expected, registers);
    }
};

TEST_F(CpuBranchTest, bpl_branch_taken) {
    registers.pc = 0xD321;
    stage_instruction(BPL);

    branch_test(0, 0x79, 3);
}

TEST_F(CpuBranchTest, bpl_crossing_page_boundary) {
    registers.pc = 0xD390;
    stage_instruction(BPL);

    branch_test(0, 0x79, 4);
}

TEST_F(CpuBranchTest, bpl_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BPL);

    branch_test(0, -128 + 5, 3);
}

TEST_F(CpuBranchTest, bmi_branch_taken) {
    registers.pc = 0xD321;
    stage_instruction(BMI);

    branch_test(N_FLAG, 0x79, 3);
}

TEST_F(CpuBranchTest, bmi_crossing_page_boundary) {
    registers.pc = 0xD390;
    stage_instruction(BMI);

    branch_test(N_FLAG, 0x79, 4);
}

TEST_F(CpuBranchTest, bmi_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BMI);

    branch_test(N_FLAG, -128 + 5, 3);
}

TEST_F(CpuBranchTest, bvc_branch_taken) {
    registers.pc = 0xA350;
    stage_instruction(BVC);

    branch_test(0, 0x10, 3);
}

TEST_F(CpuBranchTest, bvc_crossing_page_boundary) {
    registers.pc = 0xD390;
    stage_instruction(BVC);

    branch_test(0, 0x70, 4);
}

TEST_F(CpuBranchTest, bvc_negative_operand) {
    registers.pc = 0xA350;
    stage_instruction(BVC);

    branch_test(0, -128 + 10, 3);
}

TEST_F(CpuBranchTest, bvs_branch_taken) {
    registers.pc = 0xC300;
    stage_instruction(BVS);

    branch_test(V_FLAG, 127, 3);
}

TEST_F(CpuBranchTest, bvs_crossing_page_boundary) {
    registers.pc = 0xC3FD;
    stage_instruction(BVS);

    branch_test(V_FLAG, 3, 4);
}

TEST_F(CpuBranchTest, bvs_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BVS);

    branch_test(V_FLAG, -128 + 70, 3);
}

TEST_F(CpuBranchTest, bcc_branch_taken) {
    registers.pc = 0x0010;
    stage_instruction(BCC);

    branch_test(0, 0x10, 3);
}

TEST_F(CpuBranchTest, bcc_crossing_page_negative) {
    registers.pc = 0x0010;
    stage_instruction(BCC);

    branch_test(0, -70, 4);
}

TEST_F(CpuBranchTest, bcs_branch_taken) {
    registers.pc = 0xC3F0;
    stage_instruction(BCS);

    branch_test(C_FLAG, 5, 3);
}

TEST_F(CpuBranchTest, bcs_crossing_page_boundary) {
    registers.pc = 0xC3FD;
    stage_instruction(BCS);

    branch_test(C_FLAG, 3, 4);
}

TEST_F(CpuBranchTest, bcs_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BCS);

    branch_test(C_FLAG, -128 + 127, 3);
}

TEST_F(CpuBranchTest, bne_branch_taken) {
    registers.pc = 0xFF7A;
    stage_instruction(BNE);

    branch_test(0, 5, 3);
}

TEST_F(CpuBranchTest, bne_crossing_page_negative) {
    registers.pc = 0xFF7A;
    stage_instruction(BNE);

    branch_test(0, -127, 4);
}

TEST_F(CpuBranchTest, beq_branch_taken) {
    registers.pc = 0xFFFB;
    stage_instruction(BEQ);

    branch_test(Z_FLAG, 5, 3);
}

TEST_F(CpuBranchTest, beq_crossing_page_boundary) {
    registers.pc = 0xFFFB;
    stage_instruction(BEQ);

    branch_test(Z_FLAG, 125, 4);
}

TEST_F(CpuBranchTest, beq_negative_operand) {
    registers.pc = 0xD321;
    stage_instruction(BEQ);

    branch_test(Z_FLAG, -128, 3);
}

} // namespace
