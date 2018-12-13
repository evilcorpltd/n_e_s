// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/mmu_factory.h"
#include "membank.h"

#include "mmu.h"

namespace n_e_s::core {

std::unique_ptr<IMmu> MmuFactory::create(MemBankList mem_banks) {
    auto mmu = std::make_unique<Mmu>();

    for (std::unique_ptr<IMemBank> &mem_bank : mem_banks) {
        mmu->add_mem_bank(std::move(mem_bank));
    }

    return mmu;
}

} // namespace n_e_s::core
