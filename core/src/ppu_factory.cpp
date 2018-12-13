// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/ppu_factory.h"

#include "ppu.h"

namespace n_e_s::core {

std::unique_ptr<IPpu> PpuFactory::create() {
    return std::make_unique<Ppu>();
}

} // namespace n_e_s::core
