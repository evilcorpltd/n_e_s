#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class NopAbsoluteXFixture : public CpuTest,
                            public testing::WithParamInterface<uint8_t> {};

TEST_P(NopAbsoluteXFixture, nop_absx) {
    stage_instruction(GetParam());
    expected.pc += 2;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2)).WillOnce(Return(0x12));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}
INSTANTIATE_TEST_SUITE_P(NopAbsoluteX,
        NopAbsoluteXFixture,
        testing::Values(NOP_ABSX1C,
                NOP_ABSX3C,
                NOP_ABSX5C,
                NOP_ABSX7C,
                NOP_ABSXDC,
                NOP_ABSXFC));

} // namespace
