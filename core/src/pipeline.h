#pragma once

#include <deque>
#include <functional>

namespace n_e_s::core {

enum class StepResult { Continue, Skip, Stop };

class Pipeline {
    using StepT = std::function<void()>;
    using ConditionalStepT = std::function<StepResult()>;

public:
    // Pushes a step to this pipeline. The step will always be executed as long
    // as the step before was executed.
    void push(const StepT &step);

    // Pushes a step to this pipeline.
    // If the step returns Stop, then this pipeline will be considered
    // done and any remaining steps will not be exectued.
    // If the step returns Continue, then the following step will be run the
    // next time this pipeline is executed.
    // If the step returns Skip, then the followinge step will be run
    // directly.
    void push_conditional(ConditionalStepT step);

    // Pushes all steps in another pipeline to this pipeline.
    void append(const Pipeline &pipeline);

    // Returns true if this pipeline has no more steps to execute.
    bool done() const;

    void clear();
    void execute_step();

private:
    bool continue_{true};
    std::deque<ConditionalStepT> steps_;
};

} // namespace n_e_s::core
