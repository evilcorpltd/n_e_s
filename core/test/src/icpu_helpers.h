#pragma once

#include "core/icpu.h"

#include <iosfwd>

namespace n_e_s::core {

bool operator==(const ICpu::Registers &a, const ICpu::Registers &b);

void PrintTo(const ICpu::Registers &r, std::ostream *os);

} // namespace n_e_s::core
