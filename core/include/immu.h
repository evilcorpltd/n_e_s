// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include <cstdint>

class IMmu {
public:
    virtual ~IMmu() {}
    virtual uint8_t read_byte(uint16_t addr) const = 0;
};
