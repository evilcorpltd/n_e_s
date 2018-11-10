// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "mmu.h"

namespace n_e_s::core {

uint8_t Mmu::read_byte(uint16_t /*addr*/) const {
    return 0xEA; // nop
}

}
