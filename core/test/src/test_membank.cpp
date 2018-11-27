// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/membank.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

TEST(MemBank, check_if_in_range_no_mirroring) {
    MemBank<0x4000, 0x18> mem_bank;

    EXPECT_FALSE(mem_bank.is_address_in_range(0x3999));
    EXPECT_FALSE(mem_bank.is_address_in_range(0x4018));
    EXPECT_TRUE(mem_bank.is_address_in_range(0x4000));
    EXPECT_TRUE(mem_bank.is_address_in_range(0x4017));
}

TEST(MemBank, one_byte_size) {
    MemBank<0x0000> mem_bank;

    EXPECT_TRUE(mem_bank.is_address_in_range(0x0));
    EXPECT_FALSE(mem_bank.is_address_in_range(0x1));
}

TEST(MemBank, read_write_byte) {
    MemBank<0x0000, 0xFFFF> mem_bank;

    const uint8_t byte = 0xF0;

    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mem_bank.write_byte(i, byte);
        EXPECT_EQ(byte, mem_bank.read_byte(i));
    }
}

TEST(MemBank, read_write_word) {
    MemBank<0x0000, 0xFFFF> mem_bank;

    const uint16_t word = 0xF00D;

    for (uint32_t i = 0; i < 0xFFFF; i += 0x1000) {
        mem_bank.write_word(i, word);
        EXPECT_EQ(word, mem_bank.read_word(i));
    }
}

TEST(MemBank, ram_bank_mirroring) {
    MemBank<0x0000, 0x800, 0x1FFF> mem_bank;

    const std::vector<uint16_t> addrs{0x100, 0x900, 0x1100, 0x1900};
    const std::vector<uint8_t> bytes{0x1F, 0xCC, 0x01, 0xAB};

    for (uint8_t i = 0; i < addrs.size(); ++i) {
        mem_bank.write_byte(addrs[i], bytes[i]);

        for (uint16_t addr : addrs) {
            EXPECT_EQ(bytes[i], mem_bank.read_byte(addr));
        }
    }
}

TEST(MemBank, byte_order) {
    const uint8_t byte0d = 0x0D;
    const uint8_t bytef0 = 0xF0;

    MemBank<0x0000, 0x0FFFF> mem_bank;

    mem_bank.write_word(0, 0xF00D);
    EXPECT_EQ(byte0d, mem_bank.read_byte(0));
    EXPECT_EQ(bytef0, mem_bank.read_byte(1));

    mem_bank.write_byte(0, bytef0);
    mem_bank.write_byte(1, byte0d);
    EXPECT_EQ(0x0DF0, mem_bank.read_word(0));
}

} // namespace
