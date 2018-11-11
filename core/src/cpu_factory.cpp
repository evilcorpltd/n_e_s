// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/cpu_factory.h"

#include "cpu.h"

namespace n_e_s::core {

ICpu *CpuFactory::create(IMmu *mmu) {
    return new Cpu(mmu);
}

}
