#include "core/ppu_factory.h"

#include "ppu.h"

namespace n_e_s::core {

std::unique_ptr<IPpu> PpuFactory::create(PpuRegisters *registers, IMmu *mmu) {
    return std::make_unique<Ppu>(registers, mmu);
}

} // namespace n_e_s::core
