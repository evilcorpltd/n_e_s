#pragma once

#include "core/imembank.h"
#include "core/immu.h"

#include <memory>

namespace n_e_s::core {

class MmuFactory {
public:
    [[nodiscard]] static std::unique_ptr<IMmu> create(MemBankList mem_banks);
};

} // namespace n_e_s::core
