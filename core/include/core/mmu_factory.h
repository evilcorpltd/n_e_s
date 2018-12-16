// Copyright 2018 Evil Corp contributors
// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"
#include "core/membank_factory.h"

#include <memory>

namespace n_e_s::core {

class MmuFactory {
public:
    static std::unique_ptr<IMmu> create(MemBankList mem_banks);
};

} // namespace n_e_s::core
