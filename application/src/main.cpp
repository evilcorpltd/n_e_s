// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "cpu.h"
#include "mmu.h"

int main(int, char **) {
    Mmu mmu;
    CPU cpu(&mmu);
    return 0;
}
