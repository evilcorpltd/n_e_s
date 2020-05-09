#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

namespace {

class CpuMoveTest : public CpuTest {
public:
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
};

// TSX
TEST_F(CpuMoveTest, tsx) {
    stage_instruction(TSX);
    move_test(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuMoveTest, tsx_sets_n_flag) {
    stage_instruction(TSX);
    move_test_sets_n(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuMoveTest, tsx_clears_n_flag) {
    stage_instruction(TSX);
    move_test_clears_n(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuMoveTest, tsx_sets_z_flag) {
    stage_instruction(TSX);
    move_test_sets_z(&expected.x, &expected.sp, &registers.sp);
}
TEST_F(CpuMoveTest, tsx_clears_z_flag) {
    stage_instruction(TSX);
    move_test_clears_z(&expected.x, &expected.sp, &registers.sp);
}

// TYA
TEST_F(CpuMoveTest, tya) {
    stage_instruction(TYA);
    move_test(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuMoveTest, tya_sets_n_flag) {
    stage_instruction(TYA);
    move_test_sets_n(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuMoveTest, tya_clears_n_flag) {
    stage_instruction(TYA);
    move_test_clears_n(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuMoveTest, tya_sets_z_flag) {
    stage_instruction(TYA);
    move_test_sets_z(&expected.a, &expected.y, &registers.y);
}
TEST_F(CpuMoveTest, tya_clears_z_flag) {
    stage_instruction(TYA);
    move_test_clears_z(&expected.a, &expected.y, &registers.y);
}

// TAX
TEST_F(CpuMoveTest, tax) {
    stage_instruction(TAX);
    move_test(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tax_sets_n_flag) {
    stage_instruction(TAX);
    move_test_sets_n(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tax_clears_n_flag) {
    stage_instruction(TAX);
    move_test_clears_n(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tax_sets_z_flag) {
    stage_instruction(TAX);
    move_test_sets_z(&expected.x, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tax_clears_z_flag) {
    stage_instruction(TAX);
    move_test_clears_z(&expected.x, &expected.a, &registers.a);
}

// TAY
TEST_F(CpuMoveTest, tay) {
    stage_instruction(TAY);
    move_test(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tay_sets_n_flag) {
    stage_instruction(TAY);
    move_test_sets_n(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tay_clears_n_flag) {
    stage_instruction(TAY);
    move_test_clears_n(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tay_sets_z_flag) {
    stage_instruction(TAY);
    move_test_sets_z(&expected.y, &expected.a, &registers.a);
}
TEST_F(CpuMoveTest, tay_clears_z_flag) {
    stage_instruction(TAY);
    move_test_clears_z(&expected.y, &expected.a, &registers.a);
}

// TXA
TEST_F(CpuMoveTest, txa) {
    stage_instruction(TXA);
    move_test(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuMoveTest, txa_sets_n_flag) {
    stage_instruction(TXA);
    move_test_sets_n(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuMoveTest, txa_clears_n_flag) {
    stage_instruction(TXA);
    move_test_clears_n(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuMoveTest, txa_sets_z_flag) {
    stage_instruction(TXA);
    move_test_sets_z(&expected.a, &expected.x, &registers.x);
}
TEST_F(CpuMoveTest, txa_clears_z_flag) {
    stage_instruction(TXA);
    move_test_clears_z(&expected.a, &expected.x, &registers.x);
}

} // namespace
