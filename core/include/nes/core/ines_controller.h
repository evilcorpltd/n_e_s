#pragma once

namespace n_e_s::core {

class INesController {
public:
    enum class Button { A, B, Select, Start, Up, Down, Left, Right };

    virtual ~INesController() = default;

    virtual void set(Button button, bool state) = 0;
    virtual bool get(Button button) = 0;
};

} // namespace n_e_s::core