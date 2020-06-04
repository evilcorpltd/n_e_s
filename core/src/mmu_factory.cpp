#include "nes/core/mmu_factory.h"

#include "mmu.h"
#include "nes/core/imembank.h"

namespace n_e_s::core {

std::unique_ptr<IMmu> MmuFactory::create_empty() {
    return std::make_unique<Mmu>();
}

std::unique_ptr<IMmu> MmuFactory::create(MemBankList mem_banks) {
    std::unique_ptr<IMmu> immu = std::make_unique<Mmu>();
    auto mmu = static_cast<Mmu *>(immu.get());

    for (std::unique_ptr<IMemBank> &mem_bank : mem_banks) {
        mmu->add_mem_bank(std::move(mem_bank));
    }

    return immu;
}

} // namespace n_e_s::core
