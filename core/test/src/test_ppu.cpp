#include "core/invalid_address.h"
#include "core/ppu_factory.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace n_e_s::core {

bool operator==(const PpuRegisters &a, const PpuRegisters &b) {
    return a.ctrl == b.ctrl && a.mask == b.mask && a.status == b.status &&
           a.oamaddr == b.oamaddr && a.oamdata == b.oamdata &&
           a.scroll == b.scroll && a.data == b.data;
}

} // namespace n_e_s::core

namespace {

class PpuTest : public ::testing::Test {
public:
    PpuTest() : registers(), ppu(PpuFactory::create(&registers)), expected() {}

    void step_execution(uint32_t cycles) {
        for (uint32_t i = 0; i < cycles; ++i) {
            ppu->execute();
        }
    }

    PpuRegisters registers;
    std::unique_ptr<IPpu> ppu;

    PpuRegisters expected;
};

TEST_F(PpuTest, read_invalid_address) {
    EXPECT_THROW(ppu->read_byte(0x5000), InvalidAddress);
}

TEST_F(PpuTest, write_invalid_address) {
    EXPECT_THROW(ppu->write_byte(0x3000, 0xAB), InvalidAddress);
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

TEST_F(PpuTest, set_vblank_flag_during_vertical_blanking) {
    registers.status = 0x00;
    expected.status = 0x80;

    // The VBlank flag is set at the second cycle of scanline 241
    step_execution(341 * 241 + 2);

    EXPECT_EQ(expected, registers);
}

TEST_F(PpuTest, clear_vblank_flag_during_pre_render_line) {
    registers.status = 0x00;
    expected.status = 0x00;

    // The VBlank flag is cleared at the second cycle of scanline 261
    step_execution(341 * 261 + 2);

    EXPECT_EQ(expected, registers);
}

} // namespace
