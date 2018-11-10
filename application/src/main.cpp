// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"
#include "mmu.h"

using namespace n_e_s::core;

int main(int, char **) {
    Mmu mmu;
    CPU cpu(&mmu);
    return 0;
}
