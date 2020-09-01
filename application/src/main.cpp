#include "nes/nes.h"

#include <fstream>

using namespace n_e_s::nes;

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    Nes nes;
    std::ifstream fs(argv[1], std::ios::binary);
    nes.load_rom(fs);

    for (uint32_t i = 0; i < 10'000'000; ++i) {
        nes.execute();
    }

    return 0;
}
