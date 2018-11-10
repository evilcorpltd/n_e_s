// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockMmu : public IMmu {
public:
    MOCK_CONST_METHOD1(read_byte, uint8_t(uint16_t addr));
    MOCK_CONST_METHOD1(read_word, uint16_t(uint16_t addr));

    MOCK_METHOD2(write_byte, void(uint16_t addr, uint8_t byte));
    MOCK_METHOD2(write_word, void(uint16_t addr, uint16_t word));
};

}
