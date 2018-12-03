// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/membank_factory.h"

#include "membank.h"

namespace n_e_s::core {

MemBankList MemBankFactory::create_nes_mem_banks() {
    MemBankList mem_banks;

    // Ram, repeats every 0x800 byte
    mem_banks.push_back(std::make_unique<MemBank<0x0000, 0x1FFF, 0x800>>());

    // Ppu, repeats every 0x8 byte
    mem_banks.push_back(std::make_unique<MemBank<0x2000, 0x3FFF, 0x8>>());

    // Io
    mem_banks.push_back(std::make_unique<MemBank<0x4000, 0x4017, 0x18>>());

    // Io dev
    mem_banks.push_back(std::make_unique<MemBank<0x4018, 0x401F, 0x8>>());

    // Cartridge
    mem_banks.push_back(std::make_unique<MemBank<0x4020, 0xFFFF, 0xBFE0>>());

    return mem_banks;
}

} // namespace n_e_s::core
