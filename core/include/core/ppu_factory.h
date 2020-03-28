#pragma once

#include "core/immu.h"
#include "core/ippu.h"

#include <memory>

namespace n_e_s::core {

class PpuFactory {
public:
    [[nodiscard]] static std::unique_ptr<IPpu> create(PpuRegisters *registers,
            IMmu *mmu);
};

} // namespace n_e_s::core
