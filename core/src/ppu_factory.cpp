#include "core/ppu_factory.h"

#include "ppu.h"

namespace n_e_s::core {

std::unique_ptr<IPpu> PpuFactory::create(
        std::unique_ptr<PpuRegisters> registers) {
    return std::make_unique<Ppu>(std::move(registers));
}

} // namespace n_e_s::core
