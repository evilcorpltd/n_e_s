#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "core/immu.h"
#include "core/imos6502.h"
#include "core/ippu.h"
#include "core/opcode.h"
#include "nes/nes.h"

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
        return low_bits(byte) - static_cast<uint8_t>(128);
    }
    return low_bits(byte);
}

int get_arg_count(n_e_s::core::AddressMode mode) {
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
    return 0;
}

std::string get_memory_string(const n_e_s::core::Opcode &opcode,
        const uint8_t op1,
        const uint8_t op2,
        const n_e_s::nes::Nes &nes) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;

    switch (opcode.address_mode) {
    case n_e_s::core::AddressMode::Implied:
        break;
    case n_e_s::core::AddressMode::Immediate:
        ss << "#$" << std::setfill('0') << std::setw(2) << +op1;
        break;
    case n_e_s::core::AddressMode::Zeropage:
        ss << "$" << std::setfill('0') << std::setw(2) << +op1 << " = "
           << std::setfill('0') << std::setw(2) << +nes.mmu().read_byte(op1);
        break;
    case n_e_s::core::AddressMode::ZeropageX: {
        const uint8_t effective_addr = op1 + nes.cpu_registers().x;
        ss << "$" << std::setfill('0') << std::setw(2) << +op1;
        ss << ",X @ " << std::setfill('0') << std::setw(2) << +effective_addr;
        ss << " = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr);
    } break;
    case n_e_s::core::AddressMode::ZeropageY: {
        const uint8_t effective_addr = op1 + nes.cpu_registers().y;
        ss << "$" << std::setfill('0') << std::setw(2) << +op1;
        ss << ",Y @ " << std::setfill('0') << std::setw(2) << +effective_addr;
        ss << " = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr);
    } break;
    case n_e_s::core::AddressMode::Relative:
        ss << "$" << std::setfill('0') << std::setw(4)
           << to_signed(op1) + nes.cpu().state().start_pc + 2;
        break;
    case n_e_s::core::AddressMode::Accumulator:
        ss << "A";
        break;
    case n_e_s::core::AddressMode::IndexedIndirect: {
        const uint8_t midaddr = op1 + nes.cpu_registers().x;
        const uint16_t effective_addr =
                nes.mmu().read_byte(midaddr) +
                (nes.mmu().read_byte(static_cast<uint8_t>(midaddr + 1u)) << 8u);
        ss << "($" << std::setfill('0') << std::setw(2) << +op1;
        ss << ",X) @ " << std::setfill('0') << std::setw(2) << +midaddr;
        ss << " = " << std::setfill('0') << std::setw(4) << effective_addr;
        ss << " = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr);
    } break;
    case n_e_s::core::AddressMode::IndirectIndexed: {
        const uint8_t op1_plus_one =
                nes.mmu().read_byte(static_cast<uint8_t>(op1 + 1u));
        const uint16_t midaddr = nes.mmu().read_byte(op1) |
                                 static_cast<uint16_t>(op1_plus_one << 8u);
        const uint16_t effective_addr = midaddr + nes.cpu_registers().y;
        ss << "($" << std::setfill('0') << std::setw(2) << +op1 << ")";
        ss << ",Y = " << std::setfill('0') << std::setw(4) << midaddr;
        ss << " @ " << std::setfill('0') << std::setw(4) << effective_addr;
        ss << " = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr);
    } break;
    case n_e_s::core::AddressMode::Absolute:
        ss << "$" << std::setfill('0') << std::setw(2) << +op2 << std::setw(2)
           << +op1;
        if (opcode.family != n_e_s::core::Family::JSR &&
                opcode.family != n_e_s::core::Family::JMP) {
            ss << " = " << std::setfill('0') << std::setw(2)
               << +nes.mmu().read_byte(static_cast<uint16_t>(op2 << 8u) | op1);
        }
        break;
    case n_e_s::core::AddressMode::AbsoluteX: {
        const uint16_t midaddr = op1 | static_cast<uint16_t>(op2 << 8u);
        const uint16_t effective_addr = midaddr + nes.cpu_registers().x;
        ss << "$" << std::setfill('0') << std::setw(4) << +midaddr;
        ss << ",X @ " << std::setfill('0') << std::setw(4) << effective_addr;
        ss << " = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr);
    } break;
    case n_e_s::core::AddressMode::AbsoluteY: {
        const uint16_t midaddr = op1 | static_cast<uint16_t>(op2 << 8u);
        const uint16_t effective_addr = midaddr + nes.cpu_registers().y;
        ss << "$" << std::setfill('0') << std::setw(4) << +midaddr;
        ss << ",Y @ " << std::setfill('0') << std::setw(4) << effective_addr;
        ss << " = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr);
    } break;
    case n_e_s::core::AddressMode::Indirect:
        ss << "($" << std::setfill('0') << std::setw(2) << +op2 << std::setw(2)
           << +op1;
        const uint16_t effective_addr1 = static_cast<uint16_t>(op2 << 8u) | op1;
        const uint16_t effective_addr2 = static_cast<uint16_t>(op2 << 8u) |
                                         static_cast<uint8_t>(op1 + 1u);
        ss << ") = " << std::setfill('0') << std::setw(2)
           << +nes.mmu().read_byte(effective_addr2) << std::setw(2)
           << +nes.mmu().read_byte(effective_addr1);
        break;
    }
    return ss.str();
}

std::string get_reg_string(const n_e_s::core::CpuRegisters &reg) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    ss << "A:" << std::setfill('0') << std::setw(2) << +reg.a << " "
       << "X:" << std::setfill('0') << std::setw(2) << +reg.x << " "
       << "Y:" << std::setfill('0') << std::setw(2) << +reg.y << " "
       << "P:" << std::setfill('0') << std::setw(2) << +reg.p << " "
       << "SP:" << std::setfill('0') << std::setw(2) << +reg.sp;
    return ss.str();
}

std::string get_execution_string(const n_e_s::nes::Nes &nes) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;

    const n_e_s::core::IMos6502 &cpu = nes.cpu();
    const auto state = cpu.state();

    const std::uint8_t raw_opcode = nes.mmu().read_byte(state.start_pc);
    const std::uint8_t op1 = nes.mmu().read_byte(state.start_pc + 1);
    const std::uint8_t op2 = nes.mmu().read_byte(state.start_pc + 2);

    std::cout << std::hex << std::uppercase << std::setw(4) << state.start_pc
              << "  " << std::setfill('0') << std::setw(2) << +raw_opcode;

    const int arg_count = get_arg_count(state.current_opcode->address_mode);
    if (arg_count >= 1) {
        ss << " " << std::setfill('0') << std::setw(2) << +op1;
        if (arg_count >= 2) {
            ss << " " << std::setfill('0') << std::setw(2) << +op2;
        } else {
            ss << "   ";
        }
    } else {
        ss << "      ";
    }
    ss << "  " << n_e_s::core::to_string(state.current_opcode->family);

    const std::string memory_string =
            get_memory_string(*state.current_opcode, op1, op2, nes);

    ss << " " << memory_string;
    ss << std::string(28 - memory_string.length(), ' ');

    const std::string reg_string = get_reg_string(nes.cpu_registers());
    ss << reg_string << " PPU:" << std::dec << std::setfill(' ') << std::setw(3)
       << nes.ppu().cycle() - 1 << "," << std::setw(3) << nes.ppu().scanline()
       << " CYC:" << nes.current_cycle() / 3 + 7;

    return ss.str();
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Expected one argument; nestest.nes" << std::endl;
        return 1;
    }

    try {
        n_e_s::nes::Nes nes;
        nes.load_rom(argv[1]);
        nes.cpu_registers().pc = 0xC000;

        n_e_s::core::CpuState prev_state{};

        for (int i = 0; i < 1000000; ++i) {
            nes.execute();

            const auto state = nes.cpu().state();

            if (i == 0 || prev_state.start_cycle != state.start_cycle) {
                const std::string execution_string = get_execution_string(nes);
                std::cout << execution_string << std::endl;

                prev_state = state;
            }
        }
    } catch (const std::exception &e) {
        std::cout << "Expcetion: " << e.what() << std::endl;
    }
}
