// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/memport.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class MemPortTest : public ::testing::Test {
public:
    struct ReaderWriterMock {
        MOCK_METHOD1(doRead, uint8_t(uint16_t));
        MOCK_METHOD2(doWrite, void(uint16_t, uint8_t));
    };

    auto getReader() {
        return [&](uint16_t addr) { return mock_.doRead(addr); };
    }

    auto getWriter() {
        return [&](uint16_t addr, uint8_t byte) { mock_.doWrite(addr, byte); };
    }

    void expectRead(uint16_t addr) {
        EXPECT_CALL(mock_, doRead(addr));
    }

    void expectWrite(uint16_t addr, uint8_t byte) {
        EXPECT_CALL(mock_, doWrite(addr, byte));
    }

    MemPortTest() : mock_(), mem_port_(getReader(), getWriter()) {}

    ReaderWriterMock mock_;
    MemPort mem_port_;
};

TEST_F(MemPortTest, read) {
    expectRead(0x1234);

    mem_port_.read(0x1234);
}

TEST_F(MemPortTest, write) {
    expectWrite(0xABCD, 0x34);

    mem_port_.write(0xABCD, 0x34);
}

} // namespace
