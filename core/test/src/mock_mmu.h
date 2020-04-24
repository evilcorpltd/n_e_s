#pragma once

#include "nes/core/immu.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockMmu : public IMmu {
public:
    MOCK_METHOD(uint8_t, read_byte, (uint16_t addr), (const, override));
    MOCK_METHOD(void, write_byte, (uint16_t addr, uint8_t byte), (override));
};

} // namespace n_e_s::core::test
