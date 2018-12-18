#include "core/cpu_factory.h"
#include "core/membank_factory.h"
#include "core/mmu_factory.h"
#include "core/ppu_factory.h"

#include <memory>

using namespace n_e_s::core;

int main(int, char **) {
    auto ppu_registers = std::make_unique<PpuRegisters>();
    std::unique_ptr<IPpu> ppu{PpuFactory::create(std::move(ppu_registers))};

    MemBankList mem_banks = MemBankFactory::create_nes_mem_banks(ppu.get());
    std::unique_ptr<IMmu> mmu{MmuFactory::create(std::move(mem_banks))};

    Registers registers;
    std::unique_ptr<ICpu> cpu{CpuFactory::create(&registers, mmu.get())};
    (void)cpu;

    return 0;
}
