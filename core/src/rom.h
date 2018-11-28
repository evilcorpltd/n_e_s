// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/irom.h"

namespace n_e_s::core {

class Rom : public IRom {
public:
    Rom(const INesHeader& h);
};

} // namespace n_e_s::core
