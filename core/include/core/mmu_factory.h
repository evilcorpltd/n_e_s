// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"

namespace n_e_s::core {

class MmuFactory {
public:
    static IMmu *create();
};

} // namespace n_e_s::core
