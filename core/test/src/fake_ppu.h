#pragma once

#include "core/ippu.h"

namespace n_e_s::core::test {

class FakePpu : public IPpu {
    uint8_t read_byte(uint16_t) override {
        return 0;
    }
    void write_byte(uint16_t, uint8_t) override {}
    void execute() override {}
    void set_nmi_handler(const std::function<void()> &) override {}
};

} // namespace n_e_s::core::test
