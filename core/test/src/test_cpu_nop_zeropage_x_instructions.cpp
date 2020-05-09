#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

class NopZeroXFixture : public CpuTest,
                        public testing::WithParamInterface<uint8_t> {};

TEST_P(NopZeroXFixture, nop_zerox) {
    stage_instruction(GetParam());
    expected.x = registers.x = 0x01;
    expected.pc += 1;
    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0xCD));
    EXPECT_CALL(mmu, read_byte(0xCD)).WillOnce(Return(0xFD)); // Dummy read

    step_execution(4);
    EXPECT_EQ(expected, registers);
}
INSTANTIATE_TEST_SUITE_P(NopZeroX,
        NopZeroXFixture,
        testing::Values(NOP_ZEROX14,
                NOP_ZEROX34,
                NOP_ZEROX54,
                NOP_ZEROX74,
                NOP_ZEROXD4,
                NOP_ZEROXF4));

} // namespace
