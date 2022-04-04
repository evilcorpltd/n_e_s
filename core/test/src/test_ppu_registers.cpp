#include "nes/core/ppu_registers.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

TEST(RegisterUint8, set_and_get_bit) {
    Register<uint8_t> reg;
    reg.set_bit(0);
    EXPECT_TRUE(reg.is_set(0));
    EXPECT_EQ(0b0000'0001, reg.value());

    reg.set_bit(5);
    EXPECT_TRUE(reg.is_set(5));
    EXPECT_EQ(0b0010'0001, reg.value());
}

TEST(RegisterUint8, clear_bit) {
    Register<uint8_t> reg(0b1111'0000);
    reg.clear_bit(0);
    EXPECT_FALSE(reg.is_set(0));
    EXPECT_EQ(0b1111'0000, reg.value());

    reg.clear_bit(7);
    EXPECT_FALSE(reg.is_set(7));
    EXPECT_EQ(0b0111'0000, reg.value());
}

TEST(RegisterUint8, shift_left) {
    Register<uint8_t> reg;
    reg.set_bit(0);
    reg <<= 1u;
    EXPECT_EQ(0b0000'0010, reg.value());

    reg = reg << 1u;
    EXPECT_EQ(0b0000'0100, reg.value());
}

TEST(RegisterUint8, shift_right) {
    Register<uint8_t> reg(0b1000'0000);
    reg >>= 1u;
    EXPECT_EQ(0b0100'0000, reg.value());

    reg = reg >> 1u;
    EXPECT_EQ(0b0010'0000, reg.value());
}

TEST(PpuMask, render_background_returns_true_for_bit_three) {
    PpuMask mask{};
    EXPECT_FALSE(mask.render_background());

    mask = PpuMask(0b0000'1000);
    EXPECT_TRUE(mask.render_background());
}

TEST(PpuMask, render_background_left_returns_true_for_bit_one) {
    PpuMask mask{};
    EXPECT_FALSE(mask.render_background_left());

    mask = PpuMask(0b0000'0010);
    EXPECT_TRUE(mask.render_background_left());
}

TEST(PpuMask, is_rendering_enabled_returns_true_for_bit_three_and_four) {
    PpuMask mask{};
    EXPECT_FALSE(mask.is_rendering_enabled());

    mask = PpuMask(0b0000'1000);
    EXPECT_TRUE(mask.is_rendering_enabled());

    mask = PpuMask(0b0001'0000);
    EXPECT_TRUE(mask.is_rendering_enabled());
}

TEST(PpuVram, construction) {
    constexpr PpuVram kV{0xABCD};
    EXPECT_EQ(0xABCD, kV.value());

    PpuVram v2 = kV;
    EXPECT_EQ(0xABCD, v2.value());
}

TEST(PpuVram, assignment) {
    PpuVram v1{0x1234};
    constexpr PpuVram kV2{0xABCD};
    v1 = kV2;
    EXPECT_EQ(0xABCD, v1.value());
}

TEST(PpuVram, equality) {
    constexpr PpuVram kV1{0x1234};
    constexpr PpuVram kV2{0x1234};
    constexpr PpuVram kV3{0xABCD};
    EXPECT_TRUE(kV1 == kV2);
    EXPECT_TRUE(kV1 != kV3);
}

TEST(PpuVram, fine_scroll_y) {
    constexpr PpuVram kV{0b101'00'00000'00000};
    EXPECT_EQ(0b101, kV.fine_scroll_y());
}

TEST(PpuVram, set_fine_scroll_y) {
    PpuVram v{0b000'00'00000'00000};
    v.set_fine_scroll_y(0b0110'1101);
    EXPECT_EQ(0b101, v.fine_scroll_y());
}

TEST(PpuVram, coarse_scroll_y) {
    constexpr PpuVram kV{0b000'00'10101'00000};
    EXPECT_EQ(0b10101, kV.coarse_scroll_y());
}

TEST(PpuVram, set_coarse_scroll_y) {
    PpuVram v{0b000'00'00000'00000};
    v.set_coarse_scroll_y(0b0110'1101);
    EXPECT_EQ(0b01101, v.coarse_scroll_y());
}

TEST(PpuVram, coarse_scroll_x) {
    constexpr PpuVram kV{0b000'00'00000'11001};
    EXPECT_EQ(0b11001, kV.coarse_scroll_x());
}

TEST(PpuVram, set_coarse_scroll_x) {
    PpuVram v{0b000'00'00000'00000};
    v.set_coarse_scroll_x(0b0110'1101);
    EXPECT_EQ(0b01101, v.coarse_scroll_x());
}

TEST(PpuVram, set_nametable) {
    PpuVram v{0b000'00'00000'00000};
    v.set_nametable(0b0110'0011);
    EXPECT_EQ(0b000'11'00000'00000, v.value());
}

TEST(PpuVram, increase_coarse_x_increases_x) {
    PpuVram v{0b000'00'00000'11001};
    v.increase_coarse_x();
    EXPECT_EQ(0b000'00'00000'11010, v.value());
    v.increase_coarse_x();
    EXPECT_EQ(0b000'00'00000'11011, v.value());
}

TEST(PpuVram, increase_coarse_x_switches_horizontal_nametable) {
    PpuVram v{0b000'00'00000'11111};
    v.increase_coarse_x();
    EXPECT_EQ(0b000'01'00000'00000, v.value());

    v = PpuVram(0b000'01'00000'11111);
    v.increase_coarse_x();
    EXPECT_EQ(0b000'00'00000'00000, v.value());
}

TEST(PpuVram, increase_y_increases_fine_y) {
    PpuVram v{0b100'00'11001'00000};
    v.increase_y();
    EXPECT_EQ(0b101'00'11001'00000, v.value());
    v.increase_y();
    EXPECT_EQ(0b110'00'11001'00000, v.value());
}

TEST(PpuVram, increase_y_overflows_to_coarse_y) {
    PpuVram v{0b111'00'00000'00000};
    v.increase_y();
    EXPECT_EQ(0b000'00'00001'00000, v.value());
}

TEST(PpuVram, increase_y_switches_vertical_nametable_for_last_row) {
    PpuVram v{0b111'00'11101'00000}; // 0b11101 = 29 (last row)
    v.increase_y();
    EXPECT_EQ(0b000'10'00000'00000, v.value());
    v = PpuVram(0b111'10'11101'00000);
    v.increase_y();
    EXPECT_EQ(0b000'00'00000'00000, v.value());
}

TEST(PpuVram, increase_y_resets_coarse_y_for_y_out_of_bounds) {
    PpuVram v{0b111'10'11111'00000};
    v.increase_y();
    EXPECT_EQ(0b000'10'00000'00000, v.value());
}

} // namespace
