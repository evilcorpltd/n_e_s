#include "ippu_helpers.h"
#include "nes/core/invalid_address.h"
#include "nes/core/ppu_factory.h"

#include "nes/core/test/mock_mmu.h"

#include <gtest/gtest.h>
#include <array>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::NiceMock;
using testing::Return;

namespace {

constexpr int kCyclesPerScanline = 341;

class PpuTest : public ::testing::Test {
public:
    PpuTest()
            : registers(),
              ppu(PpuFactory::create(&registers, &mmu)),
              expected() {}

    void step_execution(uint32_t cycles) {
        for (uint32_t i = 0; i < cycles; ++i) {
            ppu->execute();
        }
    }

    void step_execution(uint32_t cycles, bool expect_pixel) {
        for (uint32_t i = 0; i < cycles; ++i) {
            if (expect_pixel) {
                EXPECT_TRUE(ppu->execute().has_value());
            } else {
                EXPECT_TRUE(ppu->execute().has_value());
            }
        }
    }

    PpuRegisters registers;
    NiceMock<MockMmu> mmu;
    std::unique_ptr<IPpu> ppu;

    PpuRegisters expected;
};

TEST_F(PpuTest, reads_from_mmu_for_unhandled_address) {
    EXPECT_CALL(mmu, read_byte(0x1000)).WillOnce(Return(0x56));
    EXPECT_EQ(0x56, ppu->read_byte(0x1000));
}

TEST_F(PpuTest, writes_to_mmu_for_unhandled_address) {
    EXPECT_CALL(mmu, write_byte(0x1000, 0x45)).Times(1);
    ppu->write_byte(0x1000, 0x45);
}

TEST_F(PpuTest, read_status_register_clears_write_toggle) {
    registers.status = PpuStatus(0x25);
    registers.write_toggle = true;
    expected.write_toggle = false;

    const uint8_t status = ppu->read_byte(0x2002);

    // First 5 bits is whetever is on the open bus (emtpy here)
    EXPECT_EQ(0x20, status);
}

TEST_F(PpuTest, clear_status_when_reading_status) {
    registers.status = PpuStatus(0xFF);
    expected.status = PpuStatus(0x7F);

    ppu->read_byte(0x2002);

    EXPECT_EQ(expected, registers);
}

// Test from the example show at:
// https://wiki.nesdev.org/w/index.php/PPU_scrolling#Summary
TEST_F(PpuTest, scrolling_tests) {
    registers.temp_vram_addr = PpuVram(0b00001100'00000000u);
    registers.vram_addr = PpuVram(0b00000000'00000000u);
    expected.temp_vram_addr = PpuVram(0b00000000'00000000u);
    registers.write_toggle = expected.write_toggle = true;

    ppu->write_byte(0x2000, 0x00);
    EXPECT_EQ(expected, registers);

    expected.write_toggle = false;
    ppu->read_byte(0x2002);
    EXPECT_EQ(expected, registers);

    expected.temp_vram_addr = PpuVram(0b00000000'00001111u);
    expected.fine_x_scroll = 0b101u;
    expected.write_toggle = true;
    ppu->write_byte(0x2005, 0b01111101u);
    EXPECT_EQ(expected, registers);

    expected.temp_vram_addr = PpuVram(0b01100001'01101111u);
    expected.write_toggle = false;
    ppu->write_byte(0x2005, 0b01011110u);
    EXPECT_EQ(expected, registers);

    expected.temp_vram_addr = PpuVram(0b00111101'01101111u);
    expected.write_toggle = true;
    ppu->write_byte(0x2006, 0b00111101u);
    EXPECT_EQ(expected, registers);

    expected.vram_addr = expected.temp_vram_addr =
            PpuVram(0b00111101'11110000u);
    expected.write_toggle = false;
    ppu->write_byte(0x2006, 0b11110000u);
    EXPECT_EQ(expected, registers);
}

// Test from the example shown at:
// https://wiki.nesdev.org/w/index.php/PPU_scrolling#Details
TEST_F(PpuTest, split_xy_scrolling_tests) {
    registers.temp_vram_addr = PpuVram(0b11111111'00000000u);
    expected.temp_vram_addr = PpuVram(0b00000100'00000000u);
    expected.write_toggle = true;
    ppu->write_byte(0x2006, 0x04);
    EXPECT_EQ(expected, registers);

    expected.temp_vram_addr = PpuVram(0b01100100'11100000u);
    expected.write_toggle = false;
    ppu->write_byte(0x2005, 0x3E);
    EXPECT_EQ(expected, registers);

    expected.temp_vram_addr = PpuVram(0b01100100'11101111u);
    expected.write_toggle = true;
    expected.fine_x_scroll = 0b0101u;
    ppu->write_byte(0x2005, 0x7D);
    EXPECT_EQ(expected, registers);

    expected.temp_vram_addr = PpuVram(0b01100100'11101111u);
    expected.vram_addr = expected.temp_vram_addr;
    expected.write_toggle = false;
    ppu->write_byte(0x2006, 0xEF);
    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, nmi_is_triggered_when_it_should) {
    bool triggered = false;
    registers.ctrl = expected.ctrl = PpuCtrl(0b1000'0000u);
    ppu->set_nmi_handler([&] { triggered = true; });

    // Nmi shouldn't get triggered before the start of vblanking.
    step_execution(kCyclesPerScanline * 241 + 1);
    ASSERT_FALSE(triggered);
    EXPECT_EQ(241, ppu->scanline());
    EXPECT_EQ(1, ppu->cycle());

    step_execution(1);
    ASSERT_TRUE(triggered);
    EXPECT_EQ(241, ppu->scanline());
    EXPECT_EQ(2, ppu->cycle());

    // Nmi should only be triggered on scanline 241.
    triggered = false;
    step_execution(kCyclesPerScanline);
    ASSERT_FALSE(triggered);
    EXPECT_EQ(242, ppu->scanline());
    EXPECT_EQ(2, ppu->cycle());
}

TEST_F(PpuTest, nmi_is_not_triggered_if_disabled) {
    bool triggered = false;
    registers.ctrl = expected.ctrl = PpuCtrl(0x00);
    ppu->set_nmi_handler([&] { triggered = true; });

    // Nmi shouldn't get triggered since bit 7 in ctrl is 0.
    step_execution(kCyclesPerScanline * 1000);
    ASSERT_FALSE(triggered);
}

TEST_F(PpuTest, nmi_triggered_when_enabled_during_vblank) {
    bool triggered = false;
    expected.ctrl = PpuCtrl(0b1000'0000);
    ppu->set_nmi_handler([&] { triggered = true; });

    step_execution(kCyclesPerScanline * 241 + 1);
    ASSERT_FALSE(triggered);

    step_execution(1);
    ASSERT_FALSE(triggered);

    // Trigger nmi by enabling it during vblank.
    ppu->write_byte(0x2000, 0b1000'0000);
    ASSERT_TRUE(triggered);

    triggered = false;

    // Nmi should only be triggered when the bit goes from 0 to 1.
    ppu->write_byte(0x2000, 0b1000'0000);
    ASSERT_FALSE(triggered);
}

TEST_F(PpuTest, set_vblank_flag_during_vertical_blanking) {
    registers.status = PpuStatus(0x00);
    expected.status = PpuStatus(0x80);
    expected.cycle = 2;
    expected.scanline = 241;

    // The VBlank flag is set at the second cycle of scanline 241
    step_execution(kCyclesPerScanline * 241 + 2);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, clear_vblank_flag_during_pre_render_line) {
    registers.status = PpuStatus(0x00);
    expected.status = PpuStatus(0x00);
    expected.cycle = 2;
    expected.scanline = 261;

    // The VBlank flag is cleared at the second cycle of scanline 261
    step_execution(kCyclesPerScanline * 261 + 2);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_ctrl_register) {
    expected.ctrl = PpuCtrl(0xBA);
    expected.temp_vram_addr = PpuVram(0x800);

    ppu->write_byte(0x2000, 0xBA);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_mask_register) {
    expected.mask = PpuMask(0x33);

    ppu->write_byte(0x2001, 0x33);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_oamaddr_register) {
    expected.oamaddr = 0x05;

    ppu->write_byte(0x2003, 0x05);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, ignore_oamdata_write_background_enabled) {
    registers.mask = PpuMask(0b00001000);
    registers.oamaddr = 0x02;
    expected.mask = registers.mask;
    expected.oamaddr = registers.oamaddr;

    ppu->write_byte(0x2004, 0x5A);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, ignore_oamdata_write_sprite_enabled) {
    registers.mask = PpuMask(0b00001000);
    registers.oamaddr = 0x02;
    expected.mask = registers.mask;
    expected.oamaddr = registers.oamaddr;

    ppu->write_byte(0x2004, 0x73);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, ignore_oamdata_during_pre_render_scanline) {
    registers.mask = PpuMask(0b00011000);
    registers.oamaddr = 0x02;
    registers.scanline = 261;
    registers.odd_frame = true;
    expected.mask = registers.mask;
    expected.oamaddr = registers.oamaddr;
    expected.scanline = 0;
    expected.odd_frame = false;
    // Two increases when fetching two tiles for next scanline
    expected.vram_addr = PpuVram(0x0002);

    step_execution(kCyclesPerScanline);

    ppu->write_byte(0x2004, 0x73);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_oamdata_register_rendering_disabled) {
    expected.oamaddr = 0x01;

    ppu->write_byte(0x2004, 0x77);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_oamdata_register_during_vertical_blanking) {
    expected.status = PpuStatus(0x80);
    expected.oamaddr = 0x01;
    expected.scanline = 250;

    step_execution(kCyclesPerScanline * 250);

    ppu->write_byte(0x2004, 0x21);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_and_read_oamdata_register) {
    registers.oamaddr = expected.oamaddr = 0x11;

    ppu->write_byte(0x2004, 0x35);
    ppu->write_byte(0x2003, 0x11);
    const uint8_t byte = ppu->read_byte(0x2004);

    EXPECT_EQ(expected, registers);
    EXPECT_EQ(0x35, byte);
}

TEST_F(PpuTest, write_ppu_scroll_one_time) {
    expected.fine_x_scroll = 0b110;
    expected.temp_vram_addr = PpuVram(0b0000'0000'0001'1101);
    expected.write_toggle = true;

    ppu->write_byte(0x2005, 0b1110'1110);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_scroll_two_times) {
    expected.fine_x_scroll = 0b101;
    expected.temp_vram_addr = PpuVram(0b0111'0010'1000'0111);
    expected.write_toggle = false;

    ppu->write_byte(0x2005, 0b0011'1101);
    ppu->write_byte(0x2005, 0b1010'0111);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_scroll_nametable_bits_not_overwritten) {
    expected.ctrl = PpuCtrl(0b0000'0011);
    expected.temp_vram_addr = PpuVram(0b00'1100'0001'1111);
    expected.write_toggle = true;

    ppu->write_byte(0x2000, 0b0000'0011);
    ppu->write_byte(0x2005, 0b1111'1000);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_addr_one_time) {
    registers.temp_vram_addr = PpuVram(0b0011'1111'0000'0000);
    expected.temp_vram_addr = PpuVram(0b0010'1101'0000'0000);
    expected.temp_vram_addr = PpuVram(0b0010'1101'0000'0000);
    expected.write_toggle = true;

    ppu->write_byte(0x2006, 0b0010'1101);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_addr_two_times) {
    registers.temp_vram_addr = PpuVram(0b0010'1101'1111'1111);
    expected.temp_vram_addr = PpuVram(0b0010'1101'0110'0001);
    expected.vram_addr = expected.temp_vram_addr;
    expected.write_toggle = false;

    ppu->write_byte(0x2006, 0b0010'1101);
    ppu->write_byte(0x2006, 0b0110'0001);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_addr_ignores_highest_bits) {
    expected.temp_vram_addr = PpuVram(0b0010'1101'0000'0000);
    expected.write_toggle = true;

    ppu->write_byte(0x2006, 0b1110'1101);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, increment_vram_addr_by_1_after_writing) {
    expected.vram_addr = PpuVram(0x01);

    ppu->write_byte(0x2007, 0x05);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, increment_vram_addr_by_32_after_writing) {
    registers.ctrl = expected.ctrl = PpuCtrl(0x04);
    expected.vram_addr = PpuVram(0x20);

    ppu->write_byte(0x2007, 0x05);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, read_returns_open_bus) {
    // Write something to the open bus
    ppu->write_byte(0x2001, 0b1001'0110);
    registers.status = PpuStatus(0b0111'1011);

    EXPECT_EQ(0b1001'0110, ppu->read_byte(0x2000));
    EXPECT_EQ(0b1001'0110, ppu->read_byte(0x2001));

    EXPECT_EQ(0b1001'0110, ppu->read_byte(0x2003));

    EXPECT_EQ(0b1001'0110, ppu->read_byte(0x2005));
    EXPECT_EQ(0b1001'0110, ppu->read_byte(0x2006));

    // PpuData with palette address
    // Bit 6-7 from the bus, 0-5 from memory
    EXPECT_CALL(mmu, read_byte(0x3F01)).WillOnce(Return(0b0011'1110));
    EXPECT_CALL(mmu, read_byte(0x2F01)).WillOnce(Return(0x00));
    registers.vram_addr = PpuVram(0x3F01);
    EXPECT_EQ(0b1011'1110, ppu->read_byte(0x2007));
}

TEST_F(PpuTest, read_write_oam_data_updates_open_bus) {
    // Write something to oam
    registers.oamaddr = 0x12;
    ppu->write_byte(0x2004, 0x54);
    // Open bus should be updated
    EXPECT_EQ(0x54, ppu->read_byte(0x2000));

    // Reading from oam should also update open bus
    registers.oamaddr = 0x12;
    EXPECT_EQ(0x54, ppu->read_byte(0x2004));
    EXPECT_EQ(0x54, ppu->read_byte(0x2000));
}

TEST_F(PpuTest, read_status_returns_open_bus) {
    // Write something to the open bus
    ppu->write_byte(0x2001, 0b1001'0110);
    registers.status = PpuStatus(0b0111'1011);

    // Status, bit 0-4 from the bus, 5-7 from the register
    EXPECT_EQ(0b0111'0110, ppu->read_byte(0x2002));
    // Reading will update bus
    EXPECT_EQ(0b0111'0110, ppu->read_byte(0x2000));
}

TEST_F(PpuTest, forwards_ppudata_reads_to_mmu_) {
    registers.vram_addr = PpuVram(0x4001);
    EXPECT_CALL(mmu, write_byte(0x4001, 0x05)).Times(1);

    ppu->write_byte(0x2007, 0x05);
}
TEST_F(PpuTest, read_vram_below_palette_memory_start) {
    registers.vram_addr = PpuVram(0x03);

    EXPECT_CALL(mmu, read_byte(0x03)).WillOnce(Return(0x45));
    const uint8_t first_read_byte = ppu->read_byte(0x2007);

    EXPECT_CALL(mmu, read_byte(0x04)).WillOnce(Return(0x56));
    const uint8_t second_read_byte = ppu->read_byte(0x2007);

    EXPECT_EQ(0x00, first_read_byte);
    EXPECT_EQ(0x45, second_read_byte);
}

TEST_F(PpuTest, read_from_palette_memory) {
    registers.vram_addr = PpuVram(0x3F00);

    EXPECT_CALL(mmu, read_byte(0x3F00)).WillOnce(Return(0x68));
    EXPECT_CALL(mmu, read_byte(0x2F00)).WillOnce(Return(0x31));

    uint8_t read_byte = ppu->read_byte(0x2007);

    // Last two bits is whetever is on the open bus (empty here)
    EXPECT_EQ(0x28, read_byte);

    registers.vram_addr = PpuVram(0x0100);

    EXPECT_CALL(mmu, read_byte(0x0100)).WillOnce(Return(0x11));

    read_byte = ppu->read_byte(0x2007);

    EXPECT_EQ(0x31, read_byte);
}

TEST_F(PpuTest, increment_vram_addr_by_1_after_reading) {
    expected.vram_addr = PpuVram(0x0001);

    ppu->read_byte(0x2007);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, increment_vram_addr_by_32_after_reading) {
    registers.ctrl = expected.ctrl = PpuCtrl(0x04);
    expected.vram_addr = PpuVram(0x0020);

    ppu->read_byte(0x2007);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, skips_first_cycle_on_odd_frames_when_rendering_is_enabled) {
    registers.mask = expected.mask =
            PpuMask(0b000'1000); // Enable background rendering
    registers.odd_frame = false;
    registers.cycle = kCyclesPerScanline - 1u;
    registers.scanline = 261; // pre-render scanline
    expected.odd_frame = true;
    expected.cycle = 1;
    expected.scanline = 0;

    ppu->execute();

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, render_one_pixel) {
    registers.cycle = 1;
    registers.mask = expected.mask = PpuMask(0x1E); // Enable all rendering.
    // Shift registers will be shifted left once before
    // choosing bits to be rendered.
    // Final pixel value (from second highest bits here): 1*2 + 1 = 3;
    registers.pattern_table_shifter_hi = 0b0110'0000'0000'0000u;
    registers.pattern_table_shifter_low = 0b0100'0000'0000'0000u;
    registers.attribute_table_shifter_hi = 0x0000u;
    registers.attribute_table_shifter_low = 0xFFFFu;

    // Nametable fetch
    EXPECT_CALL(mmu, read_byte(0x2000)).WillOnce(Return(0xFF));

    const auto expected_pixel =
            Pixel{.x = 0u, .y = 0u, .color = Color{8, 16, 144}}; // Second color

    // Palette index
    // Background palette 1, third color (from pixel value in tile).
    // Return second color.
    EXPECT_CALL(mmu, read_byte(0x3F00 + 1u * 4u + 3u))
            .WillRepeatedly(Return(0x02));

    const auto pixel = ppu->execute();
    EXPECT_TRUE(pixel.has_value());
    EXPECT_EQ(expected_pixel, *pixel);
}

TEST_F(PpuTest, render_one_pixel_with_fine_x_scrolling) {
    registers.cycle = 1;
    registers.mask = expected.mask = PpuMask(0x1E); // Enable all rendering.
    registers.fine_x_scroll = 7u;
    // Fine x scroll is set to 7, so the ninth bits will be used for rendering.
    // Shift registers will be shifted left once before
    // choosing bit to be rendered.
    // Final pixel value (from eighth bit here): 0*2 + 1 = 1;
    registers.pattern_table_shifter_hi = 0b0100'0000'0000'0000u;
    registers.pattern_table_shifter_low = 0b0100'0000'1000'0000u;
    registers.attribute_table_shifter_hi = 0x0000;
    registers.attribute_table_shifter_low = 0xFFFF;

    // Nametable fetch
    EXPECT_CALL(mmu, read_byte(0x2000)).WillOnce(Return(0xFF));

    const auto expected_pixel = Pixel{
            .x = 0u, .y = 0u, .color = Color{8, 16, 144}}; // Second color.

    // Palette index
    // Background palette 1, first color (from pixel value in tile).
    // Return second color.
    EXPECT_CALL(mmu, read_byte(0x3F00 + 1u * 4u + 1u))
            .WillRepeatedly(Return(0x02));

    const auto pixel = ppu->execute();
    EXPECT_TRUE(pixel.has_value());
    EXPECT_EQ(expected_pixel, *pixel);
}

TEST_F(PpuTest, render_one_tile) {
    registers.mask = expected.mask = PpuMask(0x1E); // Enable all rendering
    registers.cycle = 1;
    registers.vram_addr = PpuVram(0x0002);

    registers.pattern_table_shifter_hi = 0b1111'1111'1111'1111;
    registers.pattern_table_shifter_low = 0b0000'0000'0000'0000u;
    registers.attribute_table_shifter_hi = 0x0000;
    registers.attribute_table_shifter_low = 0xFFFF;

    {
        testing::InSequence seq;
        // Next tile
        // Nametable
        EXPECT_CALL(mmu, read_byte(0x2002)).WillOnce(Return(0x03));
        // Attribute
        EXPECT_CALL(mmu, read_byte(0x23C0)).WillOnce(Return(0x00));

        // Pattern table low
        EXPECT_CALL(mmu, read_byte(0x03 * 16u)).WillOnce(Return(0x00));
        // Pattern table high
        EXPECT_CALL(mmu, read_byte(0x03 * 16u + 8u)).WillOnce(Return(0x00));
    }

    constexpr std::array kExpectedPixels = {
            Pixel{.x = 0u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 1u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 2u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 3u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 4u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 5u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 6u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 7u, .y = 0u, .color = Color{152, 150, 152}},
            Pixel{.x = 8u, .y = 0u, .color = Color{152, 150, 152}},
    };

    // Palette index
    EXPECT_CALL(mmu, read_byte(0x3F00 + 1u * 4u + 2u))
            .WillRepeatedly(Return(0x10));

    for (int i = 0; i < 8; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_TRUE(pixel.has_value());
        const Pixel expected_pixel = kExpectedPixels.at(i);
        EXPECT_EQ(expected_pixel, *pixel);
    }

    expected.scanline = 0;
    expected.cycle = 9;
    expected.vram_addr = PpuVram(0x0003);
    expected.name_table = 0x00;
    expected.name_table_latch = 0x03;
    expected.pattern_table_shifter_hi = 0x8000;
    expected.pattern_table_latch_hi = 0x00;
    expected.pattern_table_shifter_low = 0x0000;
    expected.pattern_table_latch_low = 0x00;
    expected.attribute_table_shifter_low = 0x8000;
    expected.attribute_table_shifter_hi = 0x0000;
    expected.attribute_table_latch = 0x00;

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, visible_scanline_output_background_if_disabled) {
    registers.mask = expected.mask = PpuMask(0x00); // Disable all rendering
    registers.cycle = 1;

    EXPECT_CALL(mmu, read_byte(testing::_)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(mmu, read_byte(0x3F00)).WillRepeatedly(Return(0x05));

    for (int i = 0; i < 50; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_TRUE(pixel.has_value());
        const auto expected_color = Color{92, 0, 48}; // 5th color.
        EXPECT_EQ(expected_color, pixel->color);
    }
}

TEST_F(PpuTest, visible_scanline_output_background_left_disabled) {
    // Render background, but not first 8 pixels.
    registers.mask = expected.mask = PpuMask(0b0000'1000);
    registers.cycle = 1;
    registers.pattern_table_shifter_hi = 0b1111'1111'1111'1111;
    registers.pattern_table_shifter_low = 0b0000'0000'0000'0000u;
    registers.pattern_table_latch_hi = 0xFF;
    registers.attribute_table_shifter_hi = 0xFFFF;
    registers.attribute_table_shifter_low = 0xFFFF;
    registers.attribute_table_latch = 0xFF;

    EXPECT_CALL(mmu, read_byte(testing::_)).WillRepeatedly(Return(0xFF));
    EXPECT_CALL(mmu, read_byte(0x3F00)).WillRepeatedly(Return(0x05));
    // Third palette (from attribute)
    EXPECT_CALL(mmu, read_byte(0x3F00 + 3u * 4u + 2u))
            .WillRepeatedly(Return(0x06));

    for (int i = 0; i < 10; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_TRUE(pixel.has_value());
        // Return 5th color first 8 pixels (background from 0x3F00)
        // and 6th color the following pixels.
        const auto expected_color = i < 8 ? Color{92, 0, 48} : Color{84, 4, 0};
        EXPECT_EQ(expected_color, pixel->color);
    }
}

TEST_F(PpuTest, visible_two_sub_cycles) {
    registers.scanline = expected.scanline = 0;
    registers.mask = expected.mask =
            PpuMask(0x1E); // Enable background rendering

    expected.cycle = 17;
    // Vram should be increased at cycle 8 and 16
    expected.vram_addr = PpuVram(0x0002);
    expected.name_table = 0x02;
    expected.name_table_latch = 0x03;
    expected.pattern_table_shifter_hi = 0x4C80;
    expected.pattern_table_latch_hi = 0x99;
    expected.pattern_table_shifter_low = 0x4000;
    expected.pattern_table_latch_low = 0x80;
    expected.attribute_table_shifter_low = 0x7F80;
    expected.attribute_table_shifter_hi = 0x7F80;
    expected.attribute_table_latch = 0x03;

    // Clear scrolling
    ppu->write_byte(0x2005, 0);
    ppu->write_byte(0x2005, 0);

    {
        testing::InSequence seq;
        // Nametable on cycle 1 set to index 2
        EXPECT_CALL(mmu, read_byte(0x2000)).WillOnce(Return(0x02));

        // Attribute on cycle 3.
        // Second tile has the same attribute byte as the first.
        EXPECT_CALL(mmu, read_byte(0x23C0)).WillRepeatedly(Return(0xAB));

        // Pattern table low byte on cycle 5.
        // Each tile uses 16 bytes. So tile with index 2 starts at address 16*2.
        EXPECT_CALL(mmu, read_byte(0x02 * 16u)).WillOnce(Return(0x80));
        // High byte on cycle 7
        // Second bit plane address is 8 bytes after the first bit plane.
        EXPECT_CALL(mmu, read_byte(0x02 * 16u + 8u)).WillOnce(Return(0x99));

        // Nametable on cycle 9 set to index 3
        EXPECT_CALL(mmu, read_byte(0x2001)).WillOnce(Return(0x03));

        // Attribute on cycle 11.
        // Second tile has the same attribute byte as the first.
        EXPECT_CALL(mmu, read_byte(0x23C0)).WillRepeatedly(Return(0xAB));

        // Pattern table low byte on cycle 13.
        EXPECT_CALL(mmu, read_byte(0x03 * 16u)).WillOnce(Return(0x80));
        // Pattern table high byte on cycle 15
        EXPECT_CALL(mmu, read_byte(0x03 * 16u + 8u)).WillOnce(Return(0x99));
    }

    // Palette index
    EXPECT_CALL(mmu, read_byte(0x3F00)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(mmu, read_byte(0x3F01)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F02)).WillRepeatedly(Return(0x02));
    EXPECT_CALL(mmu, read_byte(0x3F03)).WillRepeatedly(Return(0x03));

    for (int i = 0; i < 17; ++i) {
        const auto pixel = ppu->execute();
        if (i == 0) {
            EXPECT_FALSE(pixel.has_value());
        } else {
            EXPECT_TRUE(pixel.has_value());
        }
    }

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, visible_scanline) {
    registers.scanline = 0u; // Start at visible scanline
    registers.mask = expected.mask =
            PpuMask(0b0000'1000); // Enable background rendering

    expected.cycle = 257;
    expected.scanline = 0u;
    // vram addr: yyy NN YYYYY XXXXX
    // Fine scroll should be increase once, and coarse x for each tile except
    // the last (31x).
    expected.vram_addr = PpuVram(0b001'00'00000'11111);
    expected.name_table = 0x1E;
    expected.name_table_latch = 0x1F;
    expected.pattern_table_shifter_hi = 0x8700;
    expected.pattern_table_latch_hi = 0x0F;
    expected.pattern_table_shifter_low = 0x8F00;
    expected.pattern_table_latch_low = 0x1F;
    expected.attribute_table_shifter_low = 0xFF80;
    expected.attribute_table_shifter_hi = 0x8000;
    expected.attribute_table_latch = 0x01;

    // Clear scrolling
    ppu->write_byte(0x2005, 0);
    ppu->write_byte(0x2005, 0);

    // Palette index
    EXPECT_CALL(mmu, read_byte(0x3F00)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F01)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F02)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F03)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F05)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F06)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F07)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F09)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F0A)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F0B)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F0D)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F0E)).WillRepeatedly(Return(0x01));
    EXPECT_CALL(mmu, read_byte(0x3F0F)).WillRepeatedly(Return(0x01));

    // Nametables
    for (int i = 0; i < 32; ++i) {
        EXPECT_CALL(mmu, read_byte(0x2000 + i)).WillOnce(Return(i));
    }

    // Attributes. Each address should be fetched four times since each byte
    // controlls 4x4 tiles.
    for (int i = 0; i < 8; ++i) {
        EXPECT_CALL(mmu, read_byte(0x23C0 + i))
                .Times(4)
                .WillRepeatedly(Return(i));
    }

    // Background
    for (int i = 0; i < 32; ++i) {
        EXPECT_CALL(mmu, read_byte(i * 16u)).WillOnce(Return(i));
        EXPECT_CALL(mmu, read_byte(i * 16u + 8u)).WillOnce(Return(i + 0xF0));
    }

    for (int i = 0; i <= 256; ++i) {
        const auto pixel = ppu->execute();
        if (i == 0) {
            EXPECT_FALSE(pixel.has_value());
        } else {
            EXPECT_TRUE(pixel.has_value());
        }
    }
    EXPECT_EQ(expected, registers);

    // During cycle 257 the horizontal bits should be reloaded.
    expected.vram_addr = PpuVram(0b001'00'00000'00000);
    expected.cycle = 258;

    EXPECT_FALSE(ppu->execute().has_value()); // Cycle 257
    EXPECT_EQ(expected, registers);

    // Cycle 258-320
    // During cycle 280-304 the ppu is idle
    expected.cycle = 321;

    for (int i = 258; i <= 320; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);

    // Cycle 321-336.
    // Fetch first two tiles for next scanline (fine scroll y=1)
    expected.vram_addr = PpuVram(0b001'00'00000'00010);
    expected.cycle = 337;
    expected.name_table = 0x02;
    expected.name_table_latch = 0x03;
    expected.pattern_table_shifter_hi = 0xCC80;
    expected.pattern_table_latch_hi = 0x99;
    expected.pattern_table_shifter_low = 0xC000;
    expected.pattern_table_latch_low = 0x80;
    expected.attribute_table_shifter_low = 0xFF80;
    expected.attribute_table_shifter_hi = 0x7F80;
    expected.attribute_table_latch = 0x03;

    // Nametables
    EXPECT_CALL(mmu, read_byte(0x2000)).WillOnce(Return(0x02));
    EXPECT_CALL(mmu, read_byte(0x2001)).WillOnce(Return(0x03));

    // Attributes
    EXPECT_CALL(mmu, read_byte(0x23C0)).Times(2).WillRepeatedly(Return(0xAB));

    // Background for next scanline (+1 for address)
    EXPECT_CALL(mmu, read_byte(0x02 * 16u + 1u)).WillOnce(Return(0x80));
    EXPECT_CALL(mmu, read_byte(0x02 * 16u + 8u + 1u)).WillOnce(Return(0x99));
    EXPECT_CALL(mmu, read_byte(0x03 * 16u + 1u)).WillOnce(Return(0x80));
    EXPECT_CALL(mmu, read_byte(0x03 * 16u + 8u + 1u)).WillOnce(Return(0x99));

    for (int i = 321; i <= 336; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);

    // Finally cycle 337-340.
    // Two unused nametable fetches.
    expected.scanline = 1;
    expected.cycle = 0;
    EXPECT_CALL(mmu, read_byte(0x2000 + 2)).Times(2).WillRepeatedly(Return(2));
    for (int i = 337; i <= 340; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, pre_render_two_sub_cycles) {
    registers.scanline = expected.scanline = 261; // Start at pre-render
    registers.mask = expected.mask =
            PpuMask(0b0000'1000); // Enable background rendering

    expected.cycle = 17;
    // Vram should be increased at cycle 8 and 16
    expected.vram_addr = PpuVram(0x0002);
    expected.name_table = 0x02;
    expected.name_table_latch = 0x03;
    expected.pattern_table_shifter_hi = 0x4C80;
    expected.pattern_table_latch_hi = 0x99;
    expected.pattern_table_shifter_low = 0x4000;
    expected.pattern_table_latch_low = 0x80;
    expected.attribute_table_shifter_low = 0x7F80;
    expected.attribute_table_shifter_hi = 0x7F80;
    expected.attribute_table_latch = 0x03;

    // Clear scrolling
    ppu->write_byte(0x2005, 0);
    ppu->write_byte(0x2005, 0);

    {
        testing::InSequence seq;
        // First cycle.
        // Nametable on cycle 1 set to index 2
        EXPECT_CALL(mmu, read_byte(0x2000)).WillOnce(Return(0x02));

        // Attribute on cycle 3.
        // Second tile has the same attribute byte as the first.
        EXPECT_CALL(mmu, read_byte(0x23C0)).WillRepeatedly(Return(0xAB));

        // Pattern table low byte on cycle 5.
        // Each tile uses 16 bytes. So tile with index 2 starts at address 16*2.
        EXPECT_CALL(mmu, read_byte(0x02 * 16u)).WillOnce(Return(0x80));
        // High byte on cycle 7
        // Second bit plane address is 8 bytes after the first bit plane.
        EXPECT_CALL(mmu, read_byte(0x02 * 16u + 8u)).WillOnce(Return(0x99));

        // Second cycle.
        // Nametable on cycle 9 set to index 3
        EXPECT_CALL(mmu, read_byte(0x2001)).WillOnce(Return(0x03));

        // Attribute on cycle 11.
        // Second tile has the same attribute byte as the first.
        EXPECT_CALL(mmu, read_byte(0x23C0)).WillRepeatedly(Return(0xAB));

        // Pattern table low byte on cycle 13.
        EXPECT_CALL(mmu, read_byte(0x03 * 16u)).WillOnce(Return(0x80));
        // Pattern table high byte on cycle 15
        EXPECT_CALL(mmu, read_byte(0x03 * 16u + 8u)).WillOnce(Return(0x99));
    }

    for (int i = 0; i < 17; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, pre_render_scanline) {
    registers.scanline = 261u; // Start at pre-render
    registers.odd_frame = expected.odd_frame = true;
    registers.mask = expected.mask =
            PpuMask(0b0000'1000); // Enable background rendering

    expected.cycle = 257;
    expected.scanline = 261u;
    // vram addr: yyy NN YYYYY XXXXX
    // Fine scroll should be increase once, and coarse x for each tile except
    // the last (31x).
    expected.vram_addr = PpuVram(0b001'00'00000'11111);
    expected.name_table = 0x1E;
    expected.name_table_latch = 0x1F;
    expected.pattern_table_shifter_hi = 0x8700;
    expected.pattern_table_latch_hi = 0x0F;
    expected.pattern_table_shifter_low = 0x8F00;
    expected.pattern_table_latch_low = 0x1F;
    expected.attribute_table_shifter_low = 0xFF80;
    expected.attribute_table_shifter_hi = 0x8000;
    expected.attribute_table_latch = 0x01;

    // Clear scrolling
    ppu->write_byte(0x2005, 0);
    ppu->write_byte(0x2005, 0);

    // Nametables
    for (int i = 0; i < 32; ++i) {
        EXPECT_CALL(mmu, read_byte(0x2000 + i)).WillOnce(Return(i));
    }

    // Attributes. Each address should be fetched four times since each byte
    // controlls 4x4 tiles.
    for (int i = 0; i < 8; ++i) {
        EXPECT_CALL(mmu, read_byte(0x23C0 + i))
                .Times(4)
                .WillRepeatedly(Return(i));
    }

    // Background
    for (int i = 0; i < 32; ++i) {
        EXPECT_CALL(mmu, read_byte(i * 16u)).WillOnce(Return(i));
        EXPECT_CALL(mmu, read_byte(i * 16u + 8u)).WillOnce(Return(i + 0xF0));
    }

    for (int i = 0; i <= 256; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);

    // During cycle 257 the horizontal bits should be reloaded.
    expected.vram_addr = PpuVram(0b001'00'00000'00000);
    expected.cycle = 258;

    EXPECT_FALSE(ppu->execute().has_value()); // Cycle 257
    EXPECT_EQ(expected, registers);

    // Cycle 258-320
    // During cycle 280-304 the vertical scroll bits should be reloaded.
    expected.vram_addr = PpuVram(0b000'00'00000'00000);
    expected.cycle = 321;

    for (int i = 258; i <= 320; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);

    // Cycle 321-336.
    // Fetch first two tiles for next scanline.
    expected.vram_addr = PpuVram(0b000'00'00000'00010);
    expected.cycle = 337;
    expected.name_table = 0x02;
    expected.name_table_latch = 0x03;
    expected.pattern_table_shifter_hi = 0xCC80;
    expected.pattern_table_latch_hi = 0x99;
    expected.pattern_table_shifter_low = 0xC000;
    expected.pattern_table_latch_low = 0x80;
    expected.attribute_table_shifter_low = 0xFF80;
    expected.attribute_table_shifter_hi = 0x7F80;
    expected.attribute_table_latch = 0x03;

    // Nametables
    EXPECT_CALL(mmu, read_byte(0x2000)).WillOnce(Return(0x02));
    EXPECT_CALL(mmu, read_byte(0x2001)).WillOnce(Return(0x03));

    // Attributes
    EXPECT_CALL(mmu, read_byte(0x23C0)).Times(2).WillRepeatedly(Return(0xAB));

    // Background
    EXPECT_CALL(mmu, read_byte(0x02 * 16u)).WillOnce(Return(0x80));
    EXPECT_CALL(mmu, read_byte(0x02 * 16u + 8u)).WillOnce(Return(0x99));
    EXPECT_CALL(mmu, read_byte(0x03 * 16u)).WillOnce(Return(0x80));
    EXPECT_CALL(mmu, read_byte(0x03 * 16u + 8u)).WillOnce(Return(0x99));

    for (int i = 321; i <= 336; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);

    // Finally cycle 337-340.
    // Two unused nametable fetches.
    expected.scanline = 0;
    expected.cycle = 0;
    expected.odd_frame = false;
    EXPECT_CALL(mmu, read_byte(0x2000 + 2)).Times(2).WillRepeatedly(Return(2));
    for (int i = 337; i <= 340; ++i) {
        const auto pixel = ppu->execute();
        EXPECT_FALSE(pixel.has_value());
    }
    EXPECT_EQ(expected, registers);
}

} // namespace
