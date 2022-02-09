#pragma once

#include <cstdint>

namespace n_e_s::core {

// Vram register in PPU.
// From http://wiki.nesdev.com/w/index.php/PPU_scrolling
// The 15 bit registers t and v are composed this way:
// yyy NN YYYYY XXXXX
// ||| || ||||| +++++-- coarse X scroll
// ||| || +++++-------- coarse Y scroll
// ||| ++-------------- nametable select
// +++----------------- fine Y scroll
class PpuVram {
public:
    constexpr PpuVram() noexcept = default;
    constexpr explicit PpuVram(const uint16_t value) : value_(value) {}

    constexpr bool operator==(const PpuVram &) const = default;
    constexpr bool operator!=(const PpuVram &) const = default;

    constexpr uint16_t value() const {
        return value_;
    }

    constexpr void set_fine_scroll_y(uint8_t fine_scroll_y) {
        fine_scroll_y = fine_scroll_y & 7u;
        value_ &= static_cast<uint16_t>(0b1000'1111'1111'1111);
        value_ |= static_cast<uint16_t>(fine_scroll_y << 12u);
    }

    constexpr uint8_t fine_scroll_y() const {
        return static_cast<uint8_t>(value_ >> 12u) & 0b0000'0111u;
    }

    constexpr void set_coarse_scroll_y(uint8_t coarse_scroll_y) {
        coarse_scroll_y = coarse_scroll_y & 0b0001'1111u;
        value_ &= static_cast<uint16_t>(0b1111'1100'0001'1111);
        value_ |= static_cast<uint16_t>(coarse_scroll_y << 5u);
    }

    constexpr uint8_t coarse_scroll_y() const {
        return static_cast<uint8_t>(value_ >> 5u) & 0b0001'1111u;
    }

    constexpr void set_coarse_scroll_x(uint8_t coarse_scroll_x) {
        coarse_scroll_x = coarse_scroll_x & 0b0001'1111u;
        value_ &= static_cast<uint16_t>(0b1111'1111'1110'0000);
        value_ |= coarse_scroll_x;
    }

    constexpr void set_nametable(uint8_t nametable) {
        nametable = nametable & 3u;
        value_ &= static_cast<uint16_t>(0b1111'0011'1111'1111);
        value_ |= static_cast<uint16_t>(nametable << 10u);
    }

    constexpr uint8_t coarse_scroll_x() const {
        return value_ & 0b0001'1111u;
    }

    constexpr void increase_coarse_x() {
        const auto coarse_x = coarse_scroll_x();
        if (coarse_x == 31u) {
            set_coarse_scroll_x(0u);
            value_ ^= 0x0400u; // switch horizontal nametable
        } else {
            set_coarse_scroll_x(coarse_x + 1u);
        }
    }

    constexpr void increase_y() {
        if (fine_scroll_y() < 7u) {
            set_fine_scroll_y(fine_scroll_y() + 1u);
        } else {
            set_fine_scroll_y(0u);

            const uint16_t coarse_y = coarse_scroll_y();
            if (coarse_y == 29u) {
                set_coarse_scroll_y(0u);
                value_ ^= 0x0800u; // switch vertical nametable
            } else if (coarse_y == 31u) {
                set_coarse_scroll_y(0u); // nametable not switched
            } else {
                set_coarse_scroll_y(coarse_y + 1u);
            }
        }
    }

private:
    uint16_t value_{0u};
};

struct PpuRegisters {
    uint16_t scanline;
    uint16_t cycle;
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oamaddr;
    uint8_t fine_x_scroll;
    PpuVram vram_addr;
    PpuVram temp_vram_addr;
    bool write_toggle;

    uint8_t name_table_latch;
    uint8_t name_table;

    uint8_t pattern_table_latch_low;
    uint8_t pattern_table_latch_hi;
    uint16_t pattern_table_shifter_low;
    uint16_t pattern_table_shifter_hi;

    uint8_t attribute_table_latch;
    uint16_t attribute_table_shifter_low;
    uint16_t attribute_table_shifter_hi;

    [[nodiscard]] constexpr bool is_rendering_enabled() const {
        return (mask & (1u << 3u)) || (mask & (1u << 4u));
    }
};

} // namespace n_e_s::core
