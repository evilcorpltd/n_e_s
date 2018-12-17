#pragma once

#include "core/imembank.h"

#include <memory>
#include <vector>

namespace n_e_s::core {

class IPpu;

using MemBankList = std::vector<std::unique_ptr<IMemBank>>;

class MemBankFactory {
public:
    static MemBankList create_nes_mem_banks(IPpu *ppu);
};

} // namespace n_e_s::core
