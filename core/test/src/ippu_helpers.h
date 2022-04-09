#pragma once

#include "nes/core/pixel.h"
#include "nes/core/ppu_registers.h"

#include <iosfwd>

namespace n_e_s::core {

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const PpuRegisters &r, std::ostream *os);

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const Pixel &p, std::ostream *os);

} // namespace n_e_s::core
