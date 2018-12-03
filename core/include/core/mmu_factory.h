// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"
#include "core/membank_factory.h"

namespace n_e_s::core {

class MmuFactory {
public:
    static IMmu *create(MemBankList mem_banks);
};

} // namespace n_e_s::core
