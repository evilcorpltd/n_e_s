#include "nes/core/ines_controller.h"
#include "nes/core/nes_controller_factory.h"

#include <gtest/gtest.h>
#include <memory>

using namespace n_e_s::core;

namespace {

class NesControllerTest : public ::testing::Test {
public:
    std::unique_ptr<INesController> controller{
            NesControllerFactory::create_nes_controller()};

    void expect_buttons_pressed(const bool a,
            const bool b,
            const bool select,
            const bool start,
            const bool up,
            const bool down,
            const bool left,
            const bool right) {
        EXPECT_EQ(a, controller->get(INesController::Button::A));
        EXPECT_EQ(b, controller->get(INesController::Button::B));
        EXPECT_EQ(select, controller->get(INesController::Button::Select));
        EXPECT_EQ(start, controller->get(INesController::Button::Start));
        EXPECT_EQ(up, controller->get(INesController::Button::Up));
        EXPECT_EQ(down, controller->get(INesController::Button::Down));
        EXPECT_EQ(left, controller->get(INesController::Button::Left));
        EXPECT_EQ(right, controller->get(INesController::Button::Right));
    }
};

TEST_F(NesControllerTest, set_get_initial) {
    expect_buttons_pressed(
            false, false, false, false, false, false, false, false);
}

TEST_F(NesControllerTest, set_get_individual) {
    controller->set(INesController::Button::A, true);
    expect_buttons_pressed(
            true, false, false, false, false, false, false, false);
    controller->set(INesController::Button::B, true);
    expect_buttons_pressed(
            true, true, false, false, false, false, false, false);
    controller->set(INesController::Button::Select, true);
    expect_buttons_pressed(true, true, true, false, false, false, false, false);
    controller->set(INesController::Button::Start, true);
    expect_buttons_pressed(true, true, true, true, false, false, false, false);
    controller->set(INesController::Button::Up, true);
    expect_buttons_pressed(true, true, true, true, true, false, false, false);
    controller->set(INesController::Button::Down, true);
    expect_buttons_pressed(true, true, true, true, true, true, false, false);
    controller->set(INesController::Button::Left, true);
    expect_buttons_pressed(true, true, true, true, true, true, true, false);
    controller->set(INesController::Button::Right, true);
    expect_buttons_pressed(true, true, true, true, true, true, true, true);
    controller->set(INesController::Button::A, false);
    expect_buttons_pressed(false, true, true, true, true, true, true, true);
    controller->set(INesController::Button::B, false);
    expect_buttons_pressed(false, false, true, true, true, true, true, true);
    controller->set(INesController::Button::Select, false);
    expect_buttons_pressed(false, false, false, true, true, true, true, true);
    controller->set(INesController::Button::Start, false);
    expect_buttons_pressed(false, false, false, false, true, true, true, true);
    controller->set(INesController::Button::Up, false);
    expect_buttons_pressed(false, false, false, false, false, true, true, true);
    controller->set(INesController::Button::Down, false);
    expect_buttons_pressed(
            false, false, false, false, false, false, true, true);
    controller->set(INesController::Button::Left, false);
    expect_buttons_pressed(
            false, false, false, false, false, false, false, true);
    controller->set(INesController::Button::Right, false);
    expect_buttons_pressed(
            false, false, false, false, false, false, false, false);
}

} // namespace