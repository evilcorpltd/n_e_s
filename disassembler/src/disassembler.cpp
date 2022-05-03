#include "nes/disassembler.h"

#include "nes/core/immu.h"
#include "nes/core/imos6502.h"
#include "nes/core/opcode.h"

#include <fmt/format.h>
#include <set>
#include <string>

namespace n_e_s::dis {

namespace {

constexpr bool is_negative(uint8_t byte) {
    return (byte & (1u << 7u)) != 0;
}

constexpr uint8_t low_bits(uint8_t byte) {
    return static_cast<uint8_t>(
            byte & static_cast<uint8_t>(~static_cast<uint8_t>(1u << 7u)));
}

constexpr int8_t to_signed(uint8_t byte) {
    if (is_negative(byte)) {
        return static_cast<int8_t>(low_bits(byte) - static_cast<uint8_t>(128));
    }

    // this is fine since we have already checked that the number is not
    // negative
    return static_cast<int8_t>(low_bits(byte));
}

std::uint8_t read_mmu(const n_e_s::core::IMmu &mmu, const uint16_t addr) {
    // Do not read from any ppu registers since they could have side effects.
    if (std::set<std::uint16_t>{
                0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007}
                    .contains(addr)) {
        return 0;
    }
    return mmu.read_byte(addr);
}

} // namespace

std::string disassemble(const uint16_t address,
        const n_e_s::core::IMmu &mmu,
        const n_e_s::core::CpuRegisters &reg) {
    const auto opcode = n_e_s::core::decode(mmu.read_byte(address));

    const auto family = n_e_s::core::to_string(opcode.family);
    const std::string mem_string =
            n_e_s::dis::get_memory_string(opcode, address, mmu, reg);

    if (mem_string.empty()) {
        return std::string(family);
    }
    return fmt::format("{} {}", family, mem_string);
}

std::string get_memory_string(const n_e_s::core::Opcode &opcode,
        const uint16_t address,
        const n_e_s::core::IMmu &mmu,
        const n_e_s::core::CpuRegisters &reg) {
    const uint8_t op1 = read_mmu(mmu, address + 1u);
    const uint8_t op2 = read_mmu(mmu, address + 2u);

    switch (opcode.address_mode) {
    case n_e_s::core::AddressMode::Implied:
        break;
    case n_e_s::core::AddressMode::Immediate:
        return fmt::format("#${:02X}", op1);
    case n_e_s::core::AddressMode::Zeropage:
        return fmt::format("${:02X} = {:02X}", op1, read_mmu(mmu, op1));
    case n_e_s::core::AddressMode::ZeropageX: {
        const uint8_t effective_addr = op1 + reg.x;
        return fmt::format("${:02X},X @ {:02X} = {:02X}",
                op1,
                effective_addr,
                read_mmu(mmu, effective_addr));
    }
    case n_e_s::core::AddressMode::ZeropageY: {
        const uint8_t effective_addr = op1 + reg.y;
        return fmt::format("${:02X},Y @ {:02X} = {:02X}",
                op1,
                effective_addr,
                read_mmu(mmu, effective_addr));
    }
    case n_e_s::core::AddressMode::Relative:
        return fmt::format("${:04X}", to_signed(op1) + address + 2);
    case n_e_s::core::AddressMode::Accumulator:
        return "A";
    case n_e_s::core::AddressMode::IndexedIndirect: {
        const uint8_t midaddr = op1 + reg.x;
        const uint16_t effective_addr =
                read_mmu(mmu, midaddr) +
                (read_mmu(mmu, static_cast<uint8_t>(midaddr + 1u)) << 8u);
        return fmt::format("(${:02X},X) @ {:02X} = {:04X} = {:02X}",
                op1,
                midaddr,
                effective_addr,
                read_mmu(mmu, effective_addr));
    } break;
    case n_e_s::core::AddressMode::IndirectIndexed: {
        const uint8_t op1_plus_one =
                read_mmu(mmu, static_cast<uint8_t>(op1 + 1u));
        const uint16_t midaddr =
                read_mmu(mmu, op1) | static_cast<uint16_t>(op1_plus_one << 8u);
        const uint16_t effective_addr = midaddr + reg.y;
        return fmt::format("(${:02X}),Y = {:04X} @ {:04X} = {:02X}",
                op1,
                midaddr,
                effective_addr,
                read_mmu(mmu, effective_addr));
    } break;
    case n_e_s::core::AddressMode::Absolute:
        if (opcode.family != n_e_s::core::Family::JSR &&
                opcode.family != n_e_s::core::Family::JMP) {
            const uint8_t resulting_addr =
                    read_mmu(mmu, static_cast<uint16_t>(op2 << 8u) | op1);
            return fmt::format(
                    "${:02X}{:02X} = {:02X}", op2, op1, resulting_addr);
        } else {
            return fmt::format("${:02X}{:02X}", op2, op1);
        }
    case n_e_s::core::AddressMode::AbsoluteX: {
        const uint16_t midaddr = op1 | static_cast<uint16_t>(op2 << 8u);
        const uint16_t effective_addr = midaddr + reg.x;
        return fmt::format("${:04X},X @ {:04X} = {:02X}",
                midaddr,
                effective_addr,
                read_mmu(mmu, effective_addr));
    }
    case n_e_s::core::AddressMode::AbsoluteY: {
        const uint16_t midaddr = op1 | static_cast<uint16_t>(op2 << 8u);
        const uint16_t effective_addr = midaddr + reg.y;
        return fmt::format("${:04X},Y @ {:04X} = {:02X}",
                midaddr,
                effective_addr,
                read_mmu(mmu, effective_addr));
    }
    case n_e_s::core::AddressMode::Indirect:
        const uint16_t effective_addr1 = static_cast<uint16_t>(op2 << 8u) | op1;
        const uint16_t effective_addr2 = static_cast<uint16_t>(op2 << 8u) |
                                         static_cast<uint8_t>(op1 + 1u);
        return fmt::format("(${:02X}{:02X}) = {:02X}{:02X}",
                op2,
                op1,
                read_mmu(mmu, effective_addr2),
                read_mmu(mmu, effective_addr1));
    }
    return "";
}

} // namespace n_e_s::dis
