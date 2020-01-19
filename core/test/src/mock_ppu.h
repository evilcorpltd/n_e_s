#pragma once

#include "core/ippu.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockPpu : public IPpu {
public:
    MOCK_METHOD1(read_byte, uint8_t(uint16_t addr));
    MOCK_METHOD2(write_byte, void(uint16_t addr, uint8_t byte));

    MOCK_METHOD0(execute, void());

    MOCK_METHOD1(set_nmi_handler,
            void(const std::function<void()> &nmi_handler));

    MOCK_CONST_METHOD0(scanline, uint16_t());
    MOCK_CONST_METHOD0(cycle, uint16_t());
};

} // namespace n_e_s::core::test
