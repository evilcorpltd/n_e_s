#include "nes/nes.h"

#include <gtest/gtest.h>

using namespace n_e_s::nes;

namespace {

// This works because opcode 0 does nothing.
TEST(Nes, the_cycle_count_works) {
    Nes nes;
    EXPECT_EQ(0llu, nes.current_cycle());

    nes.execute();
    EXPECT_EQ(1llu, nes.current_cycle());

    for (int i = 0; i < 9; ++i) {
        nes.execute();
    }

    EXPECT_EQ(10llu, nes.current_cycle());
}

} // namespace
