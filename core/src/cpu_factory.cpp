#include "nes/core/cpu_factory.h"

#include "mos6502.h"

namespace n_e_s::core {

std::unique_ptr<IMos6502> CpuFactory::create_mos6502(
        CpuRegisters *const registers,
        IMmu *const mmu,
        IPpu *const ppu) {
    auto cpu = std::make_unique<Mos6502>(registers, mmu);
    ppu->set_nmi_handler([ptr = cpu.get()] { ptr->set_nmi(true); });
    return cpu;
}

} // namespace n_e_s::core
