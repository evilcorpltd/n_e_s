// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/cpu_factory.h"
#include "core/mmu_factory.h"

#include <memory>

using namespace n_e_s::core;

int main(int, char **) {
    Registers registers;
    std::unique_ptr<IMmu> mmu{MmuFactory::create()};
    std::unique_ptr<ICpu> cpu{CpuFactory::create(&registers, mmu.get())};
    (void)cpu;
    return 0;
}
