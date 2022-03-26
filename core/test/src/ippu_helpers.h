#pragma once

#include "nes/core/ippu.h"

#include <iosfwd>

namespace n_e_s::core {

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const PpuRegisters &r, std::ostream *os);

} // namespace n_e_s::core
