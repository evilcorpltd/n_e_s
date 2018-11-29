// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/mmu_factory.h"
#include "membank.h"

#include "mmu.h"

namespace n_e_s::core {

IMmu *MmuFactory::create() {
    auto mmu = new Mmu();

    // Ram, repeats every 0x800 byte
    mmu->add_mem_bank(new MemBank<0x0000, 0x800, 0x1FFF>());

    // Ppu, repeates every 0x8 byte
    mmu->add_mem_bank(new MemBank<0x2000, 0x8, 0x3FFF>());

    // Io
    mmu->add_mem_bank(new MemBank<0x4000, 0x18, 0x4017>());

    // Io dev
    mmu->add_mem_bank(new MemBank<0x4018, 0x8, 0x401F>());

    // Cartridge
    mmu->add_mem_bank(new MemBank<0x4020, 0xBFE0, 0xFFFF>());

    return mmu;
}

} // namespace n_e_s::core
