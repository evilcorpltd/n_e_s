#pragma once

#include <cstdint>

namespace n_e_s::core {

class IMmu {
public:
    virtual ~IMmu() = default;

    virtual uint8_t read_byte(uint16_t addr) const = 0;
    virtual void write_byte(uint16_t addr, uint8_t byte) = 0;
};

} // namespace n_e_s::core
