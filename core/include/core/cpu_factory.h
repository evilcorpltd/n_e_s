// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/icpu.h"
#include "core/immu.h"

namespace n_e_s::core {

class CpuFactory {
public:
    static ICpu *create(Registers *registers, IMmu *mmu);
};

}
