// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockMmu : public IMmu {
public:
    MOCK_CONST_METHOD1(read_byte, uint8_t(uint16_t addr));
};

}
