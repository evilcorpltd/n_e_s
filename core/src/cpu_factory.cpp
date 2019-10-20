#include "core/cpu_factory.h"

#include "mos6502.h"

namespace n_e_s::core {

std::unique_ptr<IMos6502> CpuFactory::create_mos6502(
        CpuRegisters *const registers,
        IMmu *const mmu) {
    return std::make_unique<Mos6502>(registers, mmu);
}

} // namespace n_e_s::core
