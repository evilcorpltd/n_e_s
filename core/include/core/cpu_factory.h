#pragma once

#include "core/immu.h"
#include "core/imos6502.h"

#include <memory>

namespace n_e_s::core {

class CpuFactory {
public:
    static std::unique_ptr<IMos6502> create_mos6502(CpuRegisters *registers,
            IMmu *mmu);
};

} // namespace n_e_s::core
