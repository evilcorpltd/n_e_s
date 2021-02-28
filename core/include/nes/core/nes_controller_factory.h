#pragma once

#include "nes/core/ines_controller.h"

#include <memory>

namespace n_e_s::core {

class NesControllerFactory {
public:
    [[nodiscard]] static std::unique_ptr<INesController>
    create_nes_controller();
};

} // namespace n_e_s::core