// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "immu.h"

#include <cstdint>

class Mmu : public IMmu {
    uint8_t read_byte(uint16_t addr) const override;
};
