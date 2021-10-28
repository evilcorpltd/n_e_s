#include "nes/core/invalid_address.h"
#include "nes/core/ppu_factory.h"

#include "nes/core/test/mock_mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::NiceMock;
using testing::Return;

namespace n_e_s::core {

static bool operator==(const PpuRegisters &a, const PpuRegisters &b) {
    return a.ctrl == b.ctrl && a.mask == b.mask && a.status == b.status &&
           a.oamaddr == b.oamaddr && a.fine_x_scroll == b.fine_x_scroll &&
           a.vram_addr == b.vram_addr && a.temp_vram_addr == b.temp_vram_addr &&
           a.write_toggle == b.write_toggle;
}

} // namespace n_e_s::core

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

TEST_F(PpuTest, read_status_register) {
    registers.status = 0x25;

    const uint8_t status = ppu->read_byte(0x2002);

    EXPECT_EQ(0x25, status);
}

TEST_F(PpuTest, clear_status_when_reading_status) {
    registers.status = 0xFF;
    expected.status = 0x7F;

    ppu->read_byte(0x2002);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, nmi_is_triggered_when_it_should) {
    bool triggered = false;
    registers.ctrl = expected.ctrl = 0b1000'0000;
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
    registers.ctrl = expected.ctrl = 0x00;
    ppu->set_nmi_handler([&] { triggered = true; });

    // Nmi shouldn't get triggered since bit 7 in ctrl is 0.
    step_execution(kCyclesPerScanline * 1000);
    ASSERT_FALSE(triggered);
}

TEST_F(PpuTest, nmi_triggered_when_enabled_during_vblank) {
    bool triggered = false;
    expected.ctrl = 0b1000'0000;
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
    registers.status = 0x00;
    expected.status = 0x80;

    // The VBlank flag is set at the second cycle of scanline 241
    step_execution(kCyclesPerScanline * 241 + 2);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, clear_vblank_flag_during_pre_render_line) {
    registers.status = 0x00;
    expected.status = 0x00;

    // The VBlank flag is cleared at the second cycle of scanline 261
    step_execution(kCyclesPerScanline * 261 + 2);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_ctrl_register) {
    expected.ctrl = 0xBA;
    expected.temp_vram_addr = 0x800;

    ppu->write_byte(0x2000, 0xBA);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_mask_register) {
    expected.mask = 0x33;

    ppu->write_byte(0x2001, 0x33);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_oamaddr_register) {
    expected.oamaddr = 0x05;

    ppu->write_byte(0x2003, 0x05);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, ignore_oamdata_write_background_enabled) {
    registers.mask = 0b00001000;
    registers.oamaddr = 0x02;
    expected.mask = registers.mask;
    expected.oamaddr = registers.oamaddr;

    ppu->write_byte(0x2004, 0x5A);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, ignore_oamdata_write_sprite_enabled) {
    registers.mask = 0b00010000;
    registers.oamaddr = 0x02;
    expected.mask = registers.mask;
    expected.oamaddr = registers.oamaddr;

    ppu->write_byte(0x2004, 0x73);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, ignore_oamdata_during_pre_render_scanline) {
    registers.mask = 0b00011000;
    registers.oamaddr = 0x02;
    expected.status = 0x80;
    expected.mask = registers.mask;
    expected.oamaddr = registers.oamaddr;

    step_execution(kCyclesPerScanline * 261);

    ppu->write_byte(0x2004, 0x73);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_oamdata_register_rendering_disabled) {
    expected.oamaddr = 0x01;

    ppu->write_byte(0x2004, 0x77);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_to_oamdata_register_during_vertical_blanking) {
    expected.status = 0x80;
    expected.oamaddr = 0x01;

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
    expected.temp_vram_addr = 0b0000'0000'0001'1101;
    expected.write_toggle = true;

    ppu->write_byte(0x2005, 0b1110'1110);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_scroll_two_times) {
    expected.fine_x_scroll = 0b101;
    expected.temp_vram_addr = 0b0111'0010'1000'0111;
    expected.write_toggle = false;

    ppu->write_byte(0x2005, 0b0011'1101);
    ppu->write_byte(0x2005, 0b1010'0111);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_scroll_nametable_bits_not_overwritten) {
    expected.ctrl = 0b0000'0011;
    expected.temp_vram_addr = 0b00'1100'0001'1111;
    expected.write_toggle = true;

    ppu->write_byte(0x2000, 0b0000'0011);
    ppu->write_byte(0x2005, 0b1111'1000);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_addr_one_time) {
    expected.temp_vram_addr = 0b0010'1101'0000'0000;
    expected.write_toggle = true;

    ppu->write_byte(0x2006, 0b0010'1101);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, write_ppu_addr_two_times) {
    expected.temp_vram_addr = 0b0010'1101'0110'0001;
    expected.vram_addr = expected.temp_vram_addr;
    expected.write_toggle = false;

    ppu->write_byte(0x2006, 0b0010'1101);
    ppu->write_byte(0x2006, 0b0110'0001);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, increment_vram_addr_by_1_after_writing) {
    expected.vram_addr = 0x01;

    ppu->write_byte(0x2007, 0x05);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, increment_vram_addr_by_32_after_writing) {
    registers.ctrl = expected.ctrl = 0x04;
    expected.vram_addr = 0x20;

    ppu->write_byte(0x2007, 0x05);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, forwards_ppudata_reads_to_mmu_) {
    registers.vram_addr = 0x4001;
    EXPECT_CALL(mmu, write_byte(0x4001, 0x05)).Times(1);

    ppu->write_byte(0x2007, 0x05);
}
TEST_F(PpuTest, read_vram_below_palette_memory_start) {
    registers.vram_addr = 0x03;

    EXPECT_CALL(mmu, read_byte(0x03)).WillOnce(Return(0x45));
    const uint8_t first_read_byte = ppu->read_byte(0x2007);

    EXPECT_CALL(mmu, read_byte(0x04)).WillOnce(Return(0x56));
    const uint8_t second_read_byte = ppu->read_byte(0x2007);

    EXPECT_EQ(0x00, first_read_byte);
    EXPECT_EQ(0x45, second_read_byte);
}

TEST_F(PpuTest, read_from_palette_memory) {
    registers.vram_addr = 0x3F00;

    EXPECT_CALL(mmu, read_byte(0x3F00)).WillOnce(Return(0x68));
    EXPECT_CALL(mmu, read_byte(0x2F00)).WillOnce(Return(0x31));

    uint8_t read_byte = ppu->read_byte(0x2007);

    EXPECT_EQ(0x68, read_byte);

    registers.vram_addr = 0x0100;

    EXPECT_CALL(mmu, read_byte(0x0100)).WillOnce(Return(0x11));

    read_byte = ppu->read_byte(0x2007);

    EXPECT_EQ(0x31, read_byte);
}

TEST_F(PpuTest, increment_vram_addr_by_1_after_reading) {
    expected.vram_addr = 0x0001;

    ppu->read_byte(0x2007);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, increment_vram_addr_by_32_after_reading) {
    registers.ctrl = expected.ctrl = 0x04;
    expected.vram_addr = 0x0020;

    ppu->read_byte(0x2007);

    EXPECT_EQ(expected, registers);
}

} // namespace
