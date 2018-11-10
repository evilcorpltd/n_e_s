// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"

#include <cstdint>

namespace n_e_s::core {

class Mmu : public IMmu {
    uint8_t read_byte(uint16_t addr) const override;
};

}
