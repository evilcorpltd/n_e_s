#pragma once

#include "core/imembank.h"

#include <array>
#include <cstdint>
#include <utility>

namespace n_e_s::core {

// A memory bank is specified with a start address, end address and size.
// The object will hold an array of the given size, however the specified
// address range may be greater than the size. This is to support memory
// mirroring, meaning that the same memory may be accessed at multiple
// addresses. For a memory bank to be valid the difference between end and
// start address has to be evenly dividable by the given size.
template <uint16_t StartAddr, uint16_t EndAddr, uint16_t Size>
class MemBank : public IMemBank {
public:
    static_assert(Size > 0u, "Size must be greater than zero");
    static_assert(StartAddr <= EndAddr, "Start addr greater than end addr");
    static_assert((EndAddr - StartAddr + 1u) % Size == 0,
            "Size does not match address range");

    MemBank() : bank_() {}

    bool is_address_in_range(uint16_t addr) const override {
        return addr >= StartAddr && addr <= EndAddr;
    }

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
