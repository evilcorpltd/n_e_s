#include "core/cpu_factory.h"
#include "core/membank_factory.h"
#include "core/mmu_factory.h"
#include "core/ppu_factory.h"
#include "core/rom_factory.h"

#include <memory>

using namespace n_e_s::core;

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    PpuRegisters ppu_registers;
    std::unique_ptr<IPpu> ppu{PpuFactory::create(&ppu_registers)};

    MemBankList mem_banks = MemBankFactory::create_nes_mem_banks(ppu.get());
    std::unique_ptr<IMmu> mmu{MmuFactory::create(std::move(mem_banks))};

    Registers registers;
    std::unique_ptr<ICpu> cpu{CpuFactory::create(&registers, mmu.get())};
    (void)cpu;

    std::unique_ptr<IRom> rom{RomFactory::fromFile(argv[1])};
    (void)rom;

    return 0;
}
