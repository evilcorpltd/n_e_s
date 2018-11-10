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
    mmu.write_byte(0, byte);
    EXPECT_EQ(byte, mmu.read_byte(0));
}

TEST_F(MmuTest, word) {
    const uint16_t word = 0xF00D;
    mmu.write_word(0, word);
    EXPECT_EQ(word, mmu.read_word(0));
}
