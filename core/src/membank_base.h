#pragma once

#include "nes/core/imembank.h"

#include <cstdint>
#include <functional>

namespace n_e_s::core {

// A memory bank is specified with a start address, end address and size.
// The object will hold an array of the given size, however the specified
// address range may be greater than the size. This is to support memory
// mirroring, meaning that the same memory may be accessed at multiple
// addresses. For a memory bank to be valid the difference between end and
// start address has to be evenly dividable by the given size.
template <uint16_t StartAddr, uint16_t EndAddr, uint16_t Size>
class MemBankBase : public IMemBank {
public:
    static_assert(Size > 0u, "Size must be greater than zero");
    static_assert(StartAddr <= EndAddr, "Start addr greater than end addr");
    static_assert((EndAddr - StartAddr + 1u) % Size == 0,
            "Size does not match address range");

    bool is_address_in_range(uint16_t addr) const override {
        // Use std::greater/less_equal to work around gcc warning when StartAddr
        // is 0:
        // error: comparison is always true due to limited range of data type
        // [-Werror=type-limits]
        return std::greater_equal()(addr, StartAddr) &&
               std::less_equal()(addr, EndAddr);
    }

protected:
    MemBankBase() = default;
};

} // namespace n_e_s::core
