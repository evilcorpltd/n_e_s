// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/cpu_factory.h"

#include "cpu.h"

namespace n_e_s::core {

ICpu *CpuFactory::create(Registers *const registers, IMmu *const mmu) {
    return new Cpu(registers, mmu);
}

}
