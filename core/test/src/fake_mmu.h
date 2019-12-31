#pragma once

#include "core/immu.h"

#include <gmock/gmock.h>
#include <map>
#include <sstream>
#include <stdexcept>

namespace n_e_s::core::test {

class FakeMmu : public IMmu {
public:
    uint8_t read_byte(uint16_t addr) const override {
        if (const auto iter = memory_.find(addr); iter != memory_.end()) {
            return iter->second;
        }
        std::stringstream err;
        err << "Bad address: " << std::showbase << std::hex << +addr;
        throw std::logic_error(err.str());
    }

    uint16_t read_word(uint16_t addr) const override {
        const uint8_t low = read_byte(addr);
        const auto upper = static_cast<uint16_t>(
                read_byte(addr + static_cast<uint8_t>(1)) << 8u);
        return low | upper;
    }

    void write_byte(uint16_t addr, uint8_t byte) override {
        memory_[addr] = byte;
    }

    void write_word(uint16_t addr, uint16_t word) override {
        const auto low = static_cast<uint8_t>(word & 0xFFu);
        const auto upper = static_cast<uint8_t>(word >> 8u);
        write_byte(addr, low);
        write_byte(static_cast<uint16_t>(addr + 1), upper);
    }

private:
    std::map<uint16_t, uint8_t> memory_;
};

} // namespace n_e_s::core::test
