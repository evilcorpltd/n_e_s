#include "nes/core/imembank.h"
#include "nes/core/invalid_address.h"
#include "nes/core/membank_factory.h"
#include "nes/core/mmu_factory.h"

#include "mock_irom.h"
#include "nes/core/test/mock_membank.h"
#include "nes/core/test/mock_ppu.h"

#include <gtest/gtest.h>
#include <memory>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

namespace {

std::vector<uint16_t> get_addr_list() {
    return {0x0000, 0x1000, 0x4000};
}

class NesMmuTest : public ::testing::Test {
public:
    NesMmuTest() : ppu{}, rom{} {}

    MockPpu ppu;
    MockIRom rom;
    std::unique_ptr<IMmu> mmu{MmuFactory::create(
            MemBankFactory::create_nes_mem_banks(&ppu, &rom))};
};

class MmuTest : public ::testing::Test {
public:
    std::unique_ptr<IMmu> mmu{MmuFactory::create(MemBankList())};
};

TEST_F(NesMmuTest, read_write_byte) {
    const uint8_t byte = 0xF0;

    for (uint16_t addr : get_addr_list()) {
        mmu->write_byte(addr, byte);
        EXPECT_EQ(byte, mmu->read_byte(addr));
    }
}

TEST_F(NesMmuTest, read_write_byte_to_ppu) {
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

TEST_F(NesMmuTest, read_write_byte_io_dev_bank) {
    mmu->write_byte(0x4018, 0x33);
    EXPECT_EQ(0x33, mmu->read_byte(0x4018));
}

TEST_F(NesMmuTest, ram_bank_mirroring) {
    const std::vector<uint16_t> addrs{0x100, 0x900, 0x1100, 0x1900};
    const std::vector<uint8_t> bytes{0x1F, 0xCC, 0x01, 0xAB};

    for (unsigned int i = 0; i < addrs.size(); ++i) {
        mmu->write_byte(addrs[i], bytes[i]);

        for (uint16_t addr : addrs) {
            EXPECT_EQ(bytes[i], mmu->read_byte(addr));
        }
    }
}

TEST_F(MmuTest, read_byte_invalid_address) {
    EXPECT_THROW(mmu->read_byte(0x3333), InvalidAddress);
}

TEST_F(MmuTest, write_byte_invalid_address) {
    EXPECT_THROW(mmu->write_byte(0x1234, 0xFF), InvalidAddress);
}

TEST_F(MmuTest, set_membanks) {
    auto mem_bank = std::make_unique<MockMemBank>();

    EXPECT_CALL(*mem_bank, write_byte(1234u, 42u));
    EXPECT_CALL(*mem_bank, is_address_in_range(1234u))
            .WillOnce(testing::Return(true));

    MemBankList mem_banks;
    mem_banks.emplace_back(std::move(mem_bank));
    mmu->set_mem_banks(std::move(mem_banks));

    mmu->write_byte(1234u, 42u);
}

} // namespace
