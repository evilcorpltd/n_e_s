#include "nes/core/membank_factory.h"
#include "nes/core/mmu_factory.h"

#include "mock_irom.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class PpuMembankTest : public ::testing::Test {
public:
    testing::NiceMock<test::MockIRom> rom{};
    std::unique_ptr<IMmu> mmu{
            MmuFactory::create(MemBankFactory::create_nes_ppu_mem_banks(&rom))};
};

// Palette memory
TEST_F(PpuMembankTest, the_palette_memory_works) {
    mmu->write_byte(0x3F00, 0x42);
    EXPECT_EQ(0x42, mmu->read_byte(0x3F00));
    mmu->write_byte(0x3F1F, 0x39);
    EXPECT_EQ(0x39, mmu->read_byte(0x3F1F));
    EXPECT_EQ(0x42, mmu->read_byte(0x3F00));
}

TEST_F(PpuMembankTest, the_palette_memory_is_mirrored) {
    mmu->write_byte(0x3F00, 0x01);
    for (uint16_t addr = 0x3F00; addr < 0x4000; addr += 0x20) {
        EXPECT_EQ(0x01, mmu->read_byte(addr));
    }

    mmu->write_byte(0x3F1F, 0x07);
    for (uint16_t addr = 0x3F1F; addr < 0x4000; addr += 0x20) {
        EXPECT_EQ(0x07, mmu->read_byte(addr));
    }
}

} // namespace
