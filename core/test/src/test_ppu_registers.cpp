#include "nes/core/ppu_registers.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

TEST(PpuRegisters, is_rendering_enabled_returns_true_for_bit_three_and_four) {
    PpuRegisters r{};
    EXPECT_FALSE(r.is_rendering_enabled());

    r.mask = 0b0000'1000;
    EXPECT_TRUE(r.is_rendering_enabled());

    r.mask = 0b0001'0000;
    EXPECT_TRUE(r.is_rendering_enabled());
}

} // namespace
