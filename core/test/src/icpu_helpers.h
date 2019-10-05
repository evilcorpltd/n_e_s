#pragma once

#include "core/icpu.h"

#include <iosfwd>

namespace n_e_s::core {

bool operator==(const CpuRegisters &a, const CpuRegisters &b);

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const CpuRegisters &r, std::ostream *os);

} // namespace n_e_s::core
