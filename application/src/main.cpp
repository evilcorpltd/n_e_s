#include "nes/nes.h"

using namespace n_e_s::nes;

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    (void)argv;

    Nes nes;
    nes.reset();
    // TODO: load rom into NES

    for (uint32_t i = 0; i < 9000; ++i) {
        nes.execute();
    }

    return 0;
}
