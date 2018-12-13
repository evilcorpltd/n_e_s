// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "pipeline.h"

namespace n_e_s::core {

void Pipeline::push(StepT step) {
    steps_.push_back(std::move(step));
}

bool Pipeline::empty() const {
    return steps_.empty();
}

void Pipeline::append(const Pipeline &pipeline) {
    std::copy(pipeline.steps_.begin(),
            pipeline.steps_.end(),
            std::back_inserter(steps_));
}

void Pipeline::execute_step() {
    steps_.front()();
    steps_.pop_front();
}

void Pipeline::clear() {
    while (!steps_.empty()) {
        steps_.pop_front();
    }
}

} // namespace n_e_s::core
