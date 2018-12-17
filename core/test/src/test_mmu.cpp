#include "core/membank_factory.h"
#include "core/mmu_factory.h"

#include "mock_ppu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

namespace {

std::vector<uint16_t> get_addr_list() {
    return {0x0000,
            0x1000,
            0x4000,
            0x5000,
            0x6000,
            0x7000,
            0x8000,
            0x9000,
            0xA000,
            0xB000,
            0xC000,
            0xD000,
            0xF000};
}

class MmuTest : public ::testing::Test {
public:
    MmuTest()
            : ppu{},
              mmu{MmuFactory::create(
                      MemBankFactory::create_nes_mem_banks(&ppu))} {}

    MockPpu ppu;
    std::unique_ptr<IMmu> mmu;
};

class MmuInvalidAddressTest : public ::testing::Test {
public:
    MmuInvalidAddressTest() : mmu{MmuFactory::create(MemBankList())} {}

    std::unique_ptr<IMmu> mmu;
};

TEST_F(MmuTest, read_write_byte) {
    const uint8_t byte = 0xF0;

    for (uint16_t addr : get_addr_list()) {
        mmu->write_byte(addr, byte);
        EXPECT_EQ(byte, mmu->read_byte(addr));
    }
}

TEST_F(MmuTest, read_write_word) {
    const uint16_t word = 0xF00D;

    for (uint16_t addr : get_addr_list()) {
        mmu->write_word(addr, word);
        EXPECT_EQ(word, mmu->read_word(addr));
    }
}

TEST_F(MmuTest, read_write_byte_to_ppu) {
    const uint8_t byte = 0xAB;

    EXPECT_CALL(ppu, write_byte(0x2000, 0xAB));
    EXPECT_CALL(ppu, read_byte(0x2000)).WillOnce(testing::Return(0xAB));
    EXPECT_CALL(ppu, write_byte(0x3000, 0xAB));
    EXPECT_CALL(ppu, read_byte(0x3000)).WillOnce(testing::Return(0xAB));

    mmu->write_byte(0x2000, byte);
    EXPECT_EQ(byte, mmu->read_byte(0x2000));
    mmu->write_byte(0x3000, byte);
    EXPECT_EQ(byte, mmu->read_byte(0x3000));
}

TEST_F(MmuTest, read_write_word_to_ppu) {
    const uint16_t word = 0xF00D;

    EXPECT_CALL(ppu, write_byte(0x2000, 0x0D));
    EXPECT_CALL(ppu, read_byte(0x2000)).WillOnce(testing::Return(0x0D));
    EXPECT_CALL(ppu, write_byte(0x2001, 0xF0));
    EXPECT_CALL(ppu, read_byte(0x2001)).WillOnce(testing::Return(0xF0));
    EXPECT_CALL(ppu, write_byte(0x3000, 0x0D));
    EXPECT_CALL(ppu, read_byte(0x3000)).WillOnce(testing::Return(0x0D));
    EXPECT_CALL(ppu, write_byte(0x3001, 0xF0));
    EXPECT_CALL(ppu, read_byte(0x3001)).WillOnce(testing::Return(0xF0));

    mmu->write_word(0x2000, word);
    EXPECT_EQ(word, mmu->read_word(0x2000));
    mmu->write_word(0x3000, word);
    EXPECT_EQ(word, mmu->read_word(0x3000));
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
