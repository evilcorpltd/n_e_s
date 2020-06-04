#include "nes/core/ines_header.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

TEST(INesHeader, returns_correct_mapper) {
    INesHeader h;
    h.flags_6 = 0b0011'0000;
    h.flags_7 = 0b1010'0000;
    EXPECT_EQ(0b1010'0011, h.mapper());
}

TEST(INesHeader, returns_horizonal_mirroring) {
    INesHeader h;
    h.flags_6 = 0x00;
    EXPECT_EQ(Mirroring::Horizontal, h.mirroring());
}
TEST(INesHeader, returns_vertical_mirroring) {
    INesHeader h;
    h.flags_6 = 0x01;
    EXPECT_EQ(Mirroring::Vertical, h.mirroring());
}

} // namespace
