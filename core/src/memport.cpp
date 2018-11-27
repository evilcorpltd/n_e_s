// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/memport.h"

namespace n_e_s::core {

MemPort::MemPort(Reader reader, Writer writer)
        : reader_(reader), writer_(writer) {}

uint8_t MemPort::read(uint16_t addr) const {
    return reader_(addr);
}

void MemPort::write(uint16_t addr, uint8_t byte) {
    writer_(addr, byte);
}

} // namespace n_e_s::core