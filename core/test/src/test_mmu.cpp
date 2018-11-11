// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

class MmuTest : public ::testing::Test {
public:
    MmuTest() : mmu() {
    }

    Mmu mmu;
};

TEST_F(MmuTest, read_write_byte) {
    const uint8_t byte = 0xF0;

    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mmu.write_byte(i, byte);
        EXPECT_EQ(byte, mmu.read_byte(i));
    }
}

TEST_F(MmuTest, read_write_word) {
    const uint16_t word = 0xF00D;
    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mmu.write_word(i, word);
        EXPECT_EQ(word, mmu.read_word(i));
    }
}

TEST_F(MmuTest, ram_bank_mirroring) {
    const std::vector<uint16_t> addrs{0x100, 0x900, 0x1100, 0x1900};
    const std::vector<uint8_t> bytes{0x1F, 0xCC, 0x01, 0xAB};

    for (uint8_t i = 0; i < addrs.size(); ++i) {
        mmu.write_byte(addrs[i], bytes[i]);

        for (uint16_t addr : addrs) {
            EXPECT_EQ(bytes[i], mmu.read_byte(addr));
        }
    }
}
