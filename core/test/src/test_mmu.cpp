// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/mmu_factory.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class MmuTest : public ::testing::Test {
public:
    MmuTest() : mmu{MmuFactory::create()} {
    }

    std::unique_ptr<IMmu> mmu;
};

TEST_F(MmuTest, read_write_byte) {
    const uint8_t byte = 0xF0;

    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mmu->write_byte(i, byte);
        EXPECT_EQ(byte, mmu->read_byte(i));
    }
}

TEST_F(MmuTest, read_write_word) {
    const uint16_t word = 0xF00D;
    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mmu->write_word(i, word);
        EXPECT_EQ(word, mmu->read_word(i));
    }
}

TEST_F(MmuTest, byte_order) {
    const uint8_t byte0d = 0x0D;
    const uint8_t bytef0 = 0xF0;

    mmu->write_word(0, 0xF00D);
    EXPECT_EQ(byte0d, mmu->read_byte(0));
    EXPECT_EQ(bytef0, mmu->read_byte(1));

    mmu->write_byte(0, bytef0);
    mmu->write_byte(1, byte0d);
    EXPECT_EQ(0x0DF0, mmu->read_word(0));
}

TEST_F(MmuTest, ram_bank_mirroring) {
    const std::vector<uint16_t> addrs{0x100, 0x900, 0x1100, 0x1900};
    const std::vector<uint8_t> bytes{0x1F, 0xCC, 0x01, 0xAB};

    for (uint8_t i = 0; i < addrs.size(); ++i) {
        mmu->write_byte(addrs[i], bytes[i]);

        for (uint16_t addr : addrs) {
            EXPECT_EQ(bytes[i], mmu->read_byte(addr));
        }
    }
}

}
