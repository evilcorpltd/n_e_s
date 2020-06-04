#include "nes/nes.h"

#include <gtest/gtest.h>

using namespace n_e_s::nes;

namespace {

TEST(Nes, inital_state_is_correct) {
    Nes nes;
    EXPECT_EQ(0llu, nes.current_cycle());
}

} // namespace
