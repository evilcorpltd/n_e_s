#include "nes/core/nes_controller_factory.h"

#include "nes_controller.h"

namespace n_e_s::core {

std::unique_ptr<INesController> NesControllerFactory::create_nes_controller() {
    return std::make_unique<NesController>();
}

} // namespace n_e_s::core