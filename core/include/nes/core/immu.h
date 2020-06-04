#pragma once

#include <cstdint>

#include "nes/core/imembank.h"

namespace n_e_s::core {

class IMemBank;

class IMmu {
public:
    virtual ~IMmu() = default;

    virtual void set_mem_banks(MemBankList mem_banks) = 0;

    virtual uint8_t read_byte(uint16_t addr) const = 0;
    virtual void write_byte(uint16_t addr, uint8_t byte) = 0;
};

} // namespace n_e_s::core
