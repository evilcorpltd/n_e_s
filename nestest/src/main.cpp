#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <fmt/format.h>

#include "nes/core/immu.h"
#include "nes/core/imos6502.h"
#include "nes/core/ippu.h"
#include "nes/core/opcode.h"
#include "nes/disassembler.h"
#include "nes/nes.h"

namespace {

constexpr bool is_undocumented(const n_e_s::core::Opcode &opcode) {
    if (opcode.family == n_e_s::core::Family::NOP &&
            opcode.instruction != n_e_s::core::Instruction::NopImplied) {
        return true;
    }
    if (opcode.family == n_e_s::core::Family::LAX) {
        return true;
    }
    if (opcode.family == n_e_s::core::Family::SAX) {
        return true;
    }
    if (opcode.instruction == n_e_s::core::Instruction::SbcImmediateEB) {
        return true;
    }
    if (opcode.family == n_e_s::core::Family::DCP) {
        return true;
    }
    if (opcode.family == n_e_s::core::Family::ISB) {
        return true;
    }
    if (opcode.family == n_e_s::core::Family::SLO) {
        return true;
    }
    return false;
}

std::string get_reg_string(const n_e_s::core::CpuRegisters &reg) {
    return fmt::format("A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X}",
            reg.a,
            reg.x,
            reg.y,
            reg.p,
            reg.sp);
}

std::string get_opcode_string(const n_e_s::nes::Nes &nes) {
    const n_e_s::core::IMos6502 &cpu = nes.cpu();
    const auto state = cpu.state();
    const std::uint8_t raw_opcode = nes.mmu().read_byte(state.start_pc);
    const std::uint8_t op1 = nes.mmu().read_byte(state.start_pc + 1u);
    const std::uint8_t op2 = nes.mmu().read_byte(state.start_pc + 2u);

    std::string result =
            fmt::format("{:04X}  {:02X}", state.start_pc, raw_opcode);

    const int arg_count =
            n_e_s::dis::get_arg_count(state.current_opcode->address_mode);
    if (arg_count >= 1) {
        result += fmt::format(" {:02X}", op1);
        if (arg_count >= 2) {
            result += fmt::format(" {:02X}", op2);
        } else {
            result += fmt::format("   ");
        }
    } else {
        result += fmt::format("      ");
    }
    result += fmt::format(" {}{}",
            is_undocumented(*state.current_opcode) ? "*" : " ",
            n_e_s::core::to_string(state.current_opcode->family));

    return result;
}

std::string get_execution_string(const n_e_s::nes::Nes &nes) {
    const n_e_s::core::IMos6502 &cpu = nes.cpu();
    const auto state = cpu.state();
    const std::string opcode_string = get_opcode_string(nes);
    const std::string memory_string =
            n_e_s::dis::get_memory_string(*state.current_opcode,
                    state.start_pc,
                    nes.mmu(),
                    nes.cpu_registers());

    const std::string reg_string = get_reg_string(nes.cpu_registers());

    const std::string result = fmt::format("{} {:<28}{} PPU:{:3},{:3} CYC:{}",
            opcode_string,
            memory_string,
            reg_string,
            nes.ppu().cycle() - 1,
            nes.ppu().scanline(),
            nes.current_cycle() / 3 + 7);

    return result;
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
