// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/mmu_factory.h"

#include "mmu.h"

namespace n_e_s::core {

IMmu *MmuFactory::create() {
    return new Mmu();
}

} // namespace n_e_s::core
