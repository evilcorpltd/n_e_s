#include "pipeline.h"

namespace n_e_s::core {

void Pipeline::push(const StepT &step) {
    steps_.emplace_back([step]() {
        step();
        return StepResult::Continue;
    });
}

void Pipeline::push_conditional(ConditionalStepT step) {
    steps_.push_back(std::move(step));
}

void Pipeline::append(const Pipeline &pipeline) {
    std::copy(pipeline.steps_.begin(),
            pipeline.steps_.end(),
            std::back_inserter(steps_));
}

bool Pipeline::done() const {
    return steps_.empty() || !continue_;
}

void Pipeline::clear() {
    while (!steps_.empty()) {
        steps_.pop_front();
    }
    continue_ = true;
}

void Pipeline::execute_step() {
    if (!steps_.empty()) {
        const StepResult res = steps_.front()();
        steps_.pop_front();

        continue_ = res != StepResult::Stop;

        if (res == StepResult::Skip) {
            execute_step();
        }
    }
}

} // namespace n_e_s::core
