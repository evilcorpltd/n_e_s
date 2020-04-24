#pragma once

#include "nes/core/imembank.h"
#include "nes/core/ippu.h"

namespace n_e_s::core {

class MemBankFactory {
public:
    [[nodiscard]] static MemBankList create_nes_mem_banks(IPpu *ppu);

    [[nodiscard]] static MemBankList create_nes_ppu_mem_banks();
};

} // namespace n_e_s::core
