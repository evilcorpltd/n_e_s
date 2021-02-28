#pragma once

#include "nes/core/ines_controller.h"

#include <gmock/gmock.h>

namespace n_e_s::core::test {

class MockNesController : public INesController {
public:
    MOCK_METHOD(void, set, (Button, bool), (override));
    MOCK_METHOD(bool, get, (Button), (override));
};

} // namespace n_e_s::core::test