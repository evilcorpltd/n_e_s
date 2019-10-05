#pragma once

#include "core/icpu.h"
#include "core/immu.h"

#include <memory>

namespace n_e_s::core {

class CpuFactory {
public:
    static std::unique_ptr<ICpu> create(CpuRegisters *registers, IMmu *mmu);
};

} // namespace n_e_s::core
