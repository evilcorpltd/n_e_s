#include "core/ppu_factory.h"

#include "ppu.h"

namespace n_e_s::core {

std::unique_ptr<IPpu> PpuFactory::create(PpuRegisters *registers) {
    return std::make_unique<Ppu>(registers);
}

} // namespace n_e_s::core
