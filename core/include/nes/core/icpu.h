#pragma once

namespace n_e_s::core {

class ICpu {
public:
    virtual ~ICpu() = default;

    // Steps CPU execution forward 1 cycle.
    virtual void execute() = 0;

    // Causes the CPU to jump to its reset routine.
    virtual void reset() = 0;
};

} // namespace n_e_s::core
