#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

namespace {

class NopImpliedFixture : public CpuTest,
                          public testing::WithParamInterface<uint8_t> {};

TEST_P(NopImpliedFixture, nop_implied) {
    stage_instruction(GetParam());

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
INSTANTIATE_TEST_SUITE_P(NopImplied,
        NopImpliedFixture,
        testing::Values(NOP,
                NOP_IMP1A,
                NOP_IMP3A,
                NOP_IMP5A,
                NOP_IMP7A,
                NOP_IMPDA,
                NOP_IMPFA));

} // namespace
