#pragma once

#include "core/imembank.h"

#include <cstdint>

namespace n_e_s::core {

struct INesHeader {
    uint8_t nes[4]{'N', 'E', 'S', 26}; // NES + MS-DOS EOF
    uint8_t prg_rom_size; // in 16 KB units.
    uint8_t chr_rom_size; // in 8 KB units.
    uint8_t flags_6;
    uint8_t flags_7;
    uint8_t prg_ram_size; // in 8 KB units.
    uint8_t flags_9;
    uint8_t flags_10;
    uint8_t zeros[5]{0};
};

class IRom : public IMemBank {
public:
    IRom(const INesHeader &h) : header(h) {}
    virtual ~IRom() = default;

    INesHeader header;
};

} // namespace n_e_s::core
