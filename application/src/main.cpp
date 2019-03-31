#include "nes/nes.h"

using namespace n_e_s::nes;

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    Nes nes;
    nes.load_rom(argv[1]);

    for (uint32_t i = 0; i < 10'000'000; ++i) {
        nes.execute();
    }

    return 0;
}
