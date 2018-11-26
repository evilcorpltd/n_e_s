// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/ppu_factory.h"

#include "ppu.h"

namespace n_e_s::core {

IPpu *PpuFactory::create() {
    return new Ppu();
}

} // namespace n_e_s::core