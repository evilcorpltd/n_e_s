#pragma once

#include "nes/core/immu.h"
#include "nes/core/imos6502.h"
#include "nes/core/opcode.h"

#include <cstdlib>

namespace n_e_s::dis {

constexpr int get_arg_count(n_e_s::core::AddressMode mode) {
    switch (mode) {
    case n_e_s::core::AddressMode::Implied:
    case n_e_s::core::AddressMode::Accumulator:
        return 0;
    case n_e_s::core::AddressMode::Immediate:
    case n_e_s::core::AddressMode::Zeropage:
    case n_e_s::core::AddressMode::ZeropageX:
    case n_e_s::core::AddressMode::ZeropageY:
    case n_e_s::core::AddressMode::Relative:
    case n_e_s::core::AddressMode::IndexedIndirect:
    case n_e_s::core::AddressMode::IndirectIndexed:
        return 1;
    case n_e_s::core::AddressMode::Absolute:
    case n_e_s::core::AddressMode::AbsoluteX:
    case n_e_s::core::AddressMode::AbsoluteY:
    case n_e_s::core::AddressMode::Indirect:
        return 2;
    }
    // Should not happen
    std::abort(); // GCOVR_EXCL_LINE
}

std::string disassemble(const uint16_t address,
        const n_e_s::core::IMmu &mmu,
        const n_e_s::core::CpuRegisters &reg);

std::string get_memory_string(const n_e_s::core::Opcode &opcode,
        const uint16_t address,
        const n_e_s::core::IMmu &mmu,
        const n_e_s::core::CpuRegisters &reg);

} // namespace n_e_s::dis
