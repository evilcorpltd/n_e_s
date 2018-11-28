// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/cpu_factory.h"
#include "core/mmu_factory.h"
#include "core/ppu_factory.h"
#include "core/rom_factory.h"

#include <memory>

using namespace n_e_s::core;

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    std::unique_ptr<IMmu> mmu{MmuFactory::create()};

    Registers registers;
    std::unique_ptr<ICpu> cpu{CpuFactory::create(&registers, mmu.get())};
    std::unique_ptr<IPpu> ppu{PpuFactory::create()};
    std::unique_ptr<IRom> rom{RomFactory::fromFile(argv[1])};
    (void)cpu;
    (void)ppu;
    (void)rom;

    return 0;
}
