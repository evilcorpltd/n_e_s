#include "nes/core/membank_factory.h"
#include "nes/core/mmu_factory.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class PpuMembankTest : public ::testing::Test {
public:
    PpuMembankTest()
            : mmu{MmuFactory::create(
                      MemBankFactory::create_nes_ppu_mem_banks())} {}

    std::unique_ptr<IMmu> mmu;
};

// Nametables
TEST_F(PpuMembankTest, the_nametable_memory_works) {
    mmu->write_byte(0x2000, 0x42);
    EXPECT_EQ(0x42, mmu->read_byte(0x2000));
    mmu->write_byte(0x2FFF, 0x39);
    EXPECT_EQ(0x39, mmu->read_byte(0x2FFF));
    EXPECT_EQ(0x42, mmu->read_byte(0x2000));
}

TEST_F(PpuMembankTest, the_nametable_memory_is_mirrored) {
    const auto nametable_size = 0x400 * 4;
    mmu->write_byte(0x2000, 0x01);
    for (uint16_t addr = 0x2000; addr < 0x3EFF; addr += nametable_size) {
        EXPECT_EQ(0x01, mmu->read_byte(addr));
    }

    mmu->write_byte(0x2EFA, 0x07);
    for (uint16_t addr = 0x2EFA; addr < 0x3EFF; addr += nametable_size) {
        EXPECT_EQ(0x07, mmu->read_byte(addr));
    }

    // Mirror of nametable 3 is smaller than the size of a nametable,
    // so the memory between 0x3F00 and 0x3FFF should not be mirrored.
    for (uint16_t addr = 0x3F00; addr < 0x4000; ++addr) {
        EXPECT_EQ(0x00, mmu->read_byte(addr));
    }
}

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
