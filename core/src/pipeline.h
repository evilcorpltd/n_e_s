// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include <deque>
#include <functional>

namespace n_e_s::core {

class Pipeline {
    using StepT = std::function<void()>;

public:
    void push(StepT step);
    void append(const Pipeline &pipeline);
    bool empty() const;
    void clear();
    void execute_step();

private:
    std::deque<StepT> steps_;
};

} // namespace n_e_s::core
