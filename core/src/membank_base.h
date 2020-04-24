#pragma once

#include "nes/core/imembank.h"

#include <cstdint>

namespace n_e_s::core {

// A memory bank is specified with a start address, end address and size.
// The object will hold an array of the given size, however the specified
// address range may be greater than the size. This is to support memory
// mirroring, meaning that the same memory may be accessed at multiple
// addresses.
template <uint16_t StartAddr, uint16_t EndAddr, uint16_t Size>
class MemBankBase : public IMemBank {
public:
    static_assert(Size > 0u, "Size must be greater than zero");
    static_assert(StartAddr <= EndAddr, "Start addr greater than end addr");

    bool is_address_in_range(uint16_t addr) const override {
        return addr >= StartAddr && addr <= EndAddr;
    }

protected:
    MemBankBase() = default;
};

} // namespace n_e_s::core
