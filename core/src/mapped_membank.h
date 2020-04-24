#pragma once

#include "membank_base.h"

#include <cstdint>
#include <functional>

namespace n_e_s::core {

// A mapped membank will always forward calls to the registered
// reader and writer.
template <uint16_t StartAddr, uint16_t EndAddr, uint16_t Size>
class MappedMemBank final : public MemBankBase<StartAddr, EndAddr, Size> {
public:
    using ByteReader = std::function<uint8_t(uint16_t)>;
    using ByteWriter = std::function<void(uint16_t, uint8_t)>;

    MappedMemBank(ByteReader reader, ByteWriter writer)
            : reader_{std::move(reader)}, writer_{std::move(writer)} {}

    uint8_t read_byte(uint16_t addr) const override {
        return reader_(addr);
    }

    void write_byte(uint16_t addr, uint8_t byte) override {
        writer_(addr, byte);
    }

private:
    ByteReader reader_;
    ByteWriter writer_;
};

} // namespace n_e_s::core
