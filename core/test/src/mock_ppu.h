// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/ippu.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockPpu : public IPpu {
public:
    MOCK_CONST_METHOD1(read_byte, uint8_t(uint16_t addr));

    MOCK_METHOD2(write_byte, void(uint16_t addr, uint8_t byte));

    MOCK_METHOD0(execute, void());
};

} // namespace n_e_s::core::test
