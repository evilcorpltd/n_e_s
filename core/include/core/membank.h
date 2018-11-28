// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/memport.h"

#include <cstdint>
#include <utility>

namespace n_e_s::core {

template <uint16_t StartAddr,
        uint16_t Size = 1u,
        uint16_t EndAddr = StartAddr + (Size - 1u)>
class MemBank {
public:
    static_assert(Size > 0u, "Size must be greater than zero");
    static_assert(StartAddr <= EndAddr, "Start addr greater than end addr");

    MemBank() = default;

    void connect(MemPort *mem_port) {
        mem_port_ = mem_port;
    }

    bool is_address_in_range(uint16_t addr) const {
        return addr >= StartAddr && addr <= EndAddr;
    }

    uint8_t read_byte(uint16_t addr) const {
        if (mem_port_) {
            return mem_port_->read(addr);
        } else {
            return *get_location(addr);
        }
    }

    uint16_t read_word(uint16_t addr) const {
        return read_byte(addr) | read_byte(addr + 1) << 8;
    }

    void write_byte(uint16_t addr, uint8_t byte) {
        if (mem_port_) {
            mem_port_->write(addr, byte);
        } else {
            *get_location(addr) = byte;
        }
    }

    void write_word(uint16_t addr, uint16_t word) {
        write_byte(addr, word & 0xFF);
        write_byte(addr + 1, word >> 8);
    }

private:
    uint8_t *get_location(uint16_t addr) {
        return const_cast<uint8_t *>(std::as_const(*this).get_location(addr));
    }

    const uint8_t *get_location(uint16_t addr) const {
        addr %= Size;
        return &bank_[addr];
    }

    uint8_t bank_[Size]{};
    MemPort *mem_port_{nullptr};
};

} // namespace n_e_s::core