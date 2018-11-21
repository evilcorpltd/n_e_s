// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/cpu_factory.h"

#include "mos6502.h"

namespace n_e_s::core {

ICpu *CpuFactory::create(Registers *const registers, IMmu *const mmu) {
    return new Mos6502(registers, mmu);
}

} // namespace n_e_s::core
