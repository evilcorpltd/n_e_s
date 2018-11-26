// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include "core/immu.h"
#include "core/ippu.h"

namespace n_e_s::core {

class PpuFactory {
public:
    static IPpu *create();
};

} // namespace n_e_s::core
