// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/mmu_factory.h"
#include "membank.h"

#include "mmu.h"

namespace n_e_s::core {

IMmu *MmuFactory::create() {
    auto mmu = new Mmu();

    // Ram, repeats every 0x800 byte
    mmu->add_mem_bank(std::make_unique<MemBank<0x0000, 0x1FFF, 0x800>>());

    // Ppu, repeates every 0x8 byte
    mmu->add_mem_bank(std::make_unique<MemBank<0x2000, 0x3FFF, 0x8>>());

    // Io
    mmu->add_mem_bank(std::make_unique<MemBank<0x4000, 0x4017, 0x18>>());

    // Io dev
    mmu->add_mem_bank(std::make_unique<MemBank<0x4018, 0x401F, 0x8>>());

    // Cartridge
    mmu->add_mem_bank(std::make_unique<MemBank<0x4020, 0xFFFF, 0xBFE0>>());

    return mmu;
}

} // namespace n_e_s::core
