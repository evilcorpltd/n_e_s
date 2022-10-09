#include "nes/core/apu_factory.h"

#include "apu.h"

namespace n_e_s::core {

std::unique_ptr<IApu> ApuFactory::create() {
    return std::make_unique<Apu>();
}

} // namespace n_e_s::core