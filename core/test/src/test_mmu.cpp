// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/membank_factory.h"
#include "core/mmu_factory.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class MmuTest : public ::testing::Test {
public:
    MmuTest()
            : mmu{MmuFactory::create(MemBankFactory::create_nes_mem_banks())} {}

    std::unique_ptr<IMmu> mmu;
};

class MmuInvalidAddressTest : public ::testing::Test {
public:
    MmuInvalidAddressTest() : mmu{MmuFactory::create(MemBankList())} {}

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

TEST_F(MmuTest, read_write_byte_io_dev_bank) {
    mmu->write_byte(0x4018, 0x33);
    EXPECT_EQ(0x33, mmu->read_byte(0x4018));
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

TEST_F(MmuTest, ppu_bank_mirroring) {
    std::vector<uint16_t> addrs;
    std::vector<uint8_t> bytes;

    uint16_t addr = 0x2004;
    for (uint8_t i = 1; i <= 0x80; ++i) {
        bytes.push_back(i % 0xF);
        addrs.push_back(addr);
        addr += 0x40;
    }

    for (uint8_t i = 0; i < addrs.size(); ++i) {
        mmu->write_byte(addrs[i], bytes[i]);

        for (uint16_t addr : addrs) {
            EXPECT_EQ(bytes[i], mmu->read_byte(addr));
        }
    }
}

TEST_F(MmuInvalidAddressTest, read_byte_invalid_address) {
    try {
        mmu->read_byte(0x3333);
    } catch (const std::invalid_argument &e) {
        EXPECT_EQ(e.what(), std::string("Invalid address [0x3333]"));
    }
}

TEST_F(MmuInvalidAddressTest, read_word_invalid_address) {
    try {
        mmu->read_word(0x2244);
    } catch (const std::invalid_argument &e) {
        EXPECT_EQ(e.what(), std::string("Invalid address [0x2244]"));
    }
}

TEST_F(MmuInvalidAddressTest, write_byte_invalid_address) {
    try {
        mmu->write_byte(0x1234, 0xFF);
    } catch (const std::invalid_argument &e) {
        EXPECT_EQ(e.what(), std::string("Invalid address [0x1234]"));
    }
}

TEST_F(MmuInvalidAddressTest, write_word_invalid_address) {
    try {
        mmu->write_word(0x1111, 0xFFAA);
    } catch (const std::invalid_argument &e) {
        EXPECT_EQ(e.what(), std::string("Invalid address [0x1111]"));
    }
}

} // namespace
