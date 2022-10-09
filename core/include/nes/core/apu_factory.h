#pragma once

#include "nes/core/iapu.h"

#include <memory>

namespace n_e_s::core {
class ApuFactory {
public:
    [[nodiscard]] static std::unique_ptr<IApu> create();
};

} // namespace n_e_s::core