#pragma once

#include "nes/core/ippu.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockPpu : public IPpu {
public:
    MOCK_METHOD(uint8_t, read_byte, (uint16_t addr), (override));
    MOCK_METHOD(void, write_byte, (uint16_t addr, uint8_t byte), (override));

    MOCK_METHOD(void, execute, (), (override));

    MOCK_METHOD(void,
            set_nmi_handler,
            (const std::function<void()> &nmi_handler),
            (override));

    MOCK_METHOD(uint16_t, scanline, (), (const, override));
    MOCK_METHOD(uint16_t, cycle, (), (const, override));
};

} // namespace n_e_s::core::test
