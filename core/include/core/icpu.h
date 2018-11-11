// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

namespace n_e_s::core {

class ICpu {
public:
    virtual ~ICpu() {}

    // Runs the CPU for 1 cycle.
    virtual void execute() = 0;
};

}
