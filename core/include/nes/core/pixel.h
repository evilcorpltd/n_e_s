#pragma once

#include <cstdint>

namespace n_e_s::core {

struct Color {
    uint8_t r{0u};
    uint8_t g{0u};
    uint8_t b{0u};
};

struct Pixel {
    uint8_t x{0u};
    uint8_t y{0u};
    Color color;
};

} // namespace n_e_s::core
