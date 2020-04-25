#pragma once

#include "nes/core/imembank.h"

namespace n_e_s::core {

class IPpu;
class IRom;

class MemBankFactory {
public:
    [[nodiscard]] static MemBankList create_nes_mem_banks(IPpu *ppu, IRom *rom);

    [[nodiscard]] static MemBankList create_nes_ppu_mem_banks(IRom *rom);
};

} // namespace n_e_s::core
