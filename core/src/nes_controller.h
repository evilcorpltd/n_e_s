#pragma once

#include "nes/core/ines_controller.h"

namespace n_e_s::core {

class NesController : public INesController {
public:
    void set(Button button, bool state) override {
        switch (button) {
        case Button::A:
            a_ = state;
            break;
        case Button::B:
            b_ = state;
            break;
        case Button::Select:
            select_ = state;
            break;
        case Button::Start:
            start_ = state;
            break;
        case Button::Up:
            up_ = state;
            break;
        case Button::Down:
            down_ = state;
            break;
        case Button::Left:
            left_ = state;
            break;
        case Button::Right:
            right_ = state;
            break;
        }
    }

    bool get(Button button) override {
        switch (button) {
        case Button::A:
            return a_;
        case Button::B:
            return b_;
        case Button::Select:
            return select_;
        case Button::Start:
            return start_;
        case Button::Up:
            return up_;
        case Button::Down:
            return down_;
        case Button::Left:
            return left_;
        case Button::Right:
            return right_;
        default:
            return false;
        }
    }

private:
    bool a_{false};
    bool b_{false};
    bool select_{false};
    bool start_{false};
    bool up_{false};
    bool down_{false};
    bool left_{false};
    bool right_{false};
};

} // namespace n_e_s::core