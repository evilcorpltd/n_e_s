#include <fstream>
#include <stdexcept>
#include <string>

#include <fmt/core.h>

#include "nes/core/immu.h"
#include "nes/core/imos6502.h"
#include "nes/core/ippu.h"
#include "nes/core/opcode.h"
#include "nes/nes.h"

namespace {

void print_nametable(const n_e_s::nes::Nes &nes) {
    for (uint16_t y = 0; y < 30; ++y) {
        for (uint16_t x = 0; x < 32; ++x) {
            // $2000-$23FF  Nametable 0
            // $2400-$27FF  Nametable 1
            // $2800-$2BFF  Nametable 2
            // $2C00-$2FFF  Nametable 3
            const int nametable = 0;
            const uint16_t address = 0x2000 + 0x0400 * nametable + y * 32 + x;
            const uint8_t tile_index = nes.ppu_mmu().read_byte(address);
            fmt::print("{:02X},", tile_index);
        }
        fmt::print("\n");
    }
}

} // namespace

int main(int argc, char **argv) try {
    if (argc != 3) {
        fmt::print(stderr, "Expected two argument; <rom.nes> <cycles>\n");
        return 1;
    }

    n_e_s::nes::Nes nes;
    const std::string rom = argv[1];
    const int cycles = std::stoi(argv[2]);
    std::ifstream fs(rom, std::ios::binary);
    nes.load_rom(fs);

    fmt::print("Running rom: \"{}\"\n", rom);
    fmt::print("Cycles: \"{}\"\n", cycles);

    for (int i = 0; i < cycles; ++i) {
        nes.execute();
    }
    print_nametable(nes);
} catch (const std::exception &e) {
    fmt::print(stderr, "Exception: {}\n", e.what());
    return 1;
}
