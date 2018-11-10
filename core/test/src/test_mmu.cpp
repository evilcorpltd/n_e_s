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

TEST_F(MmuTest, byte) {
    const uint8_t byte = 0xF0;

    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mmu.write_byte(i, byte);
        EXPECT_EQ(byte, mmu.read_byte(i));
    }
}

TEST_F(MmuTest, word) {
    const uint16_t word = 0xF00D;
    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mmu.write_word(i, word);
        EXPECT_EQ(word, mmu.read_word(i));
    }
}
