#pragma once

#include "membank_base.h"

#include <array>
#include <cstdint>
#include <utility>

namespace n_e_s::core {

template <uint16_t StartAddr, uint16_t EndAddr, uint16_t Size>
class MemBank : public MemBankBase<StartAddr, EndAddr, Size> {
public:
    MemBank() : bank_() {}

    uint8_t read_byte(uint16_t addr) const override {
        return *get_location(addr);
    }

    void write_byte(uint16_t addr, uint8_t byte) override {
        *get_location(addr) = byte;
    }

private:
    uint8_t *get_location(uint16_t addr) {
        return const_cast<uint8_t *>(std::as_const(*this).get_location(addr));
    }

    const uint8_t *get_location(uint16_t addr) const {
        addr %= Size;
        return &bank_.data()[addr];
    }

    std::array<uint8_t, Size> bank_;
};

} // namespace n_e_s::core
