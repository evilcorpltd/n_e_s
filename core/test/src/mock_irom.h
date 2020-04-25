#pragma once

#include "nes/core/irom.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockIRom : public IRom {
public:
    MockIRom() : IRom(INesHeader()) {}

    MOCK_METHOD(bool,
            is_cpu_address_in_range,
            (uint16_t addr),
            (const, override));
    MOCK_METHOD(uint8_t, cpu_read_byte, (uint16_t addr), (const, override));
    MOCK_METHOD(void,
            cpu_write_byte,
            (uint16_t addr, uint8_t byte),
            (override));

    MOCK_METHOD(bool,
            is_ppu_address_in_range,
            (uint16_t addr),
            (const, override));
    MOCK_METHOD(uint8_t, ppu_read_byte, (uint16_t addr), (const, override));
    MOCK_METHOD(void,
            ppu_write_byte,
            (uint16_t addr, uint8_t byte),
            (override));
};

} // namespace n_e_s::core::test
