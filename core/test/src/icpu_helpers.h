#pragma once

#include "core/icpu.h"

#include <iosfwd>

namespace n_e_s::core {

bool operator==(const ICpu::Registers &a, const ICpu::Registers &b);

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const ICpu::Registers &r, std::ostream *os);

} // namespace n_e_s::core
