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
        const uint16_t upper = read_byte(addr + 1) << 8;
        return low | upper;
    }

    void write_byte(uint16_t addr, uint8_t byte) override {
        memory_[addr] = byte;
    }

    void write_word(uint16_t addr, uint16_t word) override {
        const uint8_t low = word & 0xFF;
        const uint8_t upper = word >> 8;
        write_byte(addr, low);
        write_byte(addr + 1, upper);
    }

    std::map<uint16_t, uint8_t> memory_;
};

} // namespace n_e_s::core::test
