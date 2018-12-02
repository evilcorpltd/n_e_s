// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/imembank.h"

#include <memory>
#include <vector>

namespace n_e_s::core {

using MemBankList = std::vector<std::unique_ptr<IMemBank>>;

class MemBankFactory {
public:
    static MemBankList create_nes_mem_banks();
};

} // namespace n_e_s::core
