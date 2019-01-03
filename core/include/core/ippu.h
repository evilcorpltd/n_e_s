#pragma once

#include <cstdint>

namespace n_e_s::core {

class IPpu {
public:
    virtual ~IPpu() = default;

    virtual uint8_t read_byte(uint16_t addr) = 0;
    virtual void write_byte(uint16_t addr, uint8_t byte) = 0;

    virtual void execute() = 0;

    struct Registers {
        uint8_t ctrl;
        uint8_t mask;
        uint8_t status;
        uint8_t oamaddr;
        uint8_t oamdata;
        uint8_t scroll;
        uint8_t data;
    };
};

} // namespace n_e_s::core
