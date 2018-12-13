// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include <deque>
#include <functional>

namespace n_e_s::core {

class Pipeline {
    using StepT = std::function<void()>;

public:
    void push(StepT step);

    bool empty() const;

    void append(const Pipeline &pipeline);

    void execute_step();

    void clear();

private:
    std::deque<StepT> steps_;
};

} // namespace n_e_s::core
