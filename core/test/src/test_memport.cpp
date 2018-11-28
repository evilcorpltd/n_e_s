// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/memport.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class MemPortTest : public ::testing::Test {
public:
    struct FakeReaderWriter {
        uint8_t doRead(uint16_t addr) {
            read_addr = addr;
            return return_byte;
        }

        void doWrite(uint16_t addr, uint8_t byte) {
            write_addr = addr;
            written_byte = byte;
        }

        uint16_t read_addr{0};
        uint16_t write_addr{0};
        uint8_t return_byte{0};
        uint8_t written_byte{0};
    };

    auto getReader() {
        return [&](uint16_t addr) { return fake_rw_.doRead(addr); };
    }

    auto getWriter() {
        return [&](uint16_t addr, uint8_t byte) { fake_rw_.doWrite(addr, byte); };
    }

    MemPortTest() : fake_rw_(), mem_port_(getReader(), getWriter()) {}

    FakeReaderWriter fake_rw_;
    MemPort mem_port_;
};

TEST_F(MemPortTest, read) {
    fake_rw_.return_byte = 0x45;

    EXPECT_EQ(0x45, mem_port_.read(0x1234));
    EXPECT_EQ(0x1234, fake_rw_.read_addr);
}

TEST_F(MemPortTest, write) {
    mem_port_.write(0xABCD, 0x34);

    EXPECT_EQ(0xABCD, fake_rw_.write_addr);
    EXPECT_EQ(0x34, fake_rw_.written_byte);
}

} // namespace
