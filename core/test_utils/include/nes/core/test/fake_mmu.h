#pragma once

#include "nes/core/immu.h"

#include <gmock/gmock.h>
#include <map>
#include <sstream>
#include <stdexcept>

namespace n_e_s::core::test {

class FakeMmu : public IMmu {
public:
    void set_mem_banks(MemBankList) override {}

    uint8_t read_byte(uint16_t addr) const override {
        if (const auto iter = memory_.find(addr); iter != memory_.end()) {
            return iter->second;
        }
        std::stringstream err;
        err << "Bad address: " << std::showbase << std::hex << +addr;
        throw std::logic_error(err.str());
    }

    void write_byte(uint16_t addr, uint8_t byte) override {
        memory_[addr] = byte;
    }

private:
    std::map<uint16_t, uint8_t> memory_;
};

} // namespace n_e_s::core::test
