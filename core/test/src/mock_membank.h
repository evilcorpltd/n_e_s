// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/imembank.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockMemBank : public IMemBank {
public:
    MOCK_CONST_METHOD1(is_address_in_range, bool(uint16_t));

    MOCK_CONST_METHOD1(read_byte, uint8_t(uint16_t addr));

    MOCK_METHOD2(write_byte, void(uint16_t addr, uint8_t byte));
};

} // namespace n_e_s::core::test
