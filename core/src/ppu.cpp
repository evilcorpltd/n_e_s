#include "ppu.h"

#include <array>
#include <tuple>

#include "nes/core/invalid_address.h"
#include "nes/core/pixel.h"

namespace {

using n_e_s::core::Color;

// 2C02 VGA Palette from:
// https://wiki.nesdev.org/w/index.php?title=PPU_palettes#2C02
constexpr std::array kPalette{
        Color{84, 84, 84},
        Color{0, 30, 116},
        Color{8, 16, 144},
        Color{48, 0, 136},
        Color{68, 0, 100},
        Color{92, 0, 48},
        Color{84, 4, 0},
        Color{60, 24, 0},
        Color{32, 42, 0},
        Color{8, 58, 0},
        Color{0, 64, 0},
        Color{0, 60, 0},
        Color{0, 50, 60},
        Color{0, 0, 0},
        Color{0, 0, 0},
        Color{0, 0, 0},

        Color{152, 150, 152}, // 10
        Color{8, 76, 196},
        Color{48, 50, 236},
        Color{92, 30, 228},
        Color{136, 20, 176},
        Color{160, 20, 100},
        Color{152, 34, 32},
        Color{120, 60, 0},
        Color{84, 90, 0},
        Color{40, 114, 0},
        Color{8, 124, 0},
        Color{0, 118, 40},
        Color{0, 102, 120},
        Color{0, 0, 0},
        Color{0, 0, 0},
        Color{0, 0, 0},

        Color{236, 238, 236}, // 20
        Color{76, 154, 236},
        Color{120, 124, 236},
        Color{176, 98, 236},
        Color{228, 84, 236},
        Color{236, 88, 180},
        Color{236, 106, 100},
        Color{212, 136, 32},
        Color{160, 170, 0},
        Color{116, 196, 0},
        Color{76, 208, 32},
        Color{56, 204, 108},
        Color{56, 180, 204},
        Color{60, 60, 60},
        Color{0, 0, 0},
        Color{0, 0, 0},

        Color{236, 238, 236}, // 30
        Color{168, 204, 236},
        Color{188, 188, 236},
        Color{212, 178, 236},
        Color{236, 174, 236},
        Color{236, 174, 212},
        Color{236, 180, 176},
        Color{228, 196, 144},
        Color{204, 210, 120},
        Color{180, 222, 120},
        Color{168, 226, 144},
        Color{152, 226, 180},
        Color{160, 214, 228},
        Color{160, 162, 160},
        Color{0, 0, 0},
        Color{0, 0, 0},
};

static_assert(kPalette.size() == 64u);

const uint16_t kPpuCtrl = 0x2000;
const uint16_t kPpuMask = 0x2001;
const uint16_t kPpuStatus = 0x2002;
const uint16_t kOamAddr = 0x2003;
const uint16_t kOamData = 0x2004;
const uint16_t kPpuScroll = 0x2005;
const uint16_t kPpuAddr = 0x2006;
const uint16_t kPpuData = 0x2007;
const uint16_t kFirstPaletteData = 0x3F00;

const uint16_t kLastCycleInScanline = 340;
const uint16_t kLastScanlineInFrame = 261;
const uint16_t kPreRenderScanline = 261;
const uint16_t kVisibleScanlineEnd = 239;
const uint16_t kPostRenderScanline = 240;
const uint16_t kVBlankScanlineStart = 241;
const uint16_t kVBlankScanlineEnd = 260;

// From
// https://wiki.nesdev.org/w/index.php/PPU_scrolling#Tile_and_attribute_fetching
uint16_t get_nametable_address(uint16_t vram_addr) {
    return 0x2000u | (vram_addr & 0x0FFFu);
}

// From
// https://wiki.nesdev.org/w/index.php/PPU_scrolling#Tile_and_attribute_fetching
constexpr uint16_t get_attribute_address(uint16_t vram_addr) {
    return 0x23C0u | (vram_addr & 0x0C00u) |
           (static_cast<uint16_t>(vram_addr >> 4u) & 0x38u) |
           (static_cast<uint16_t>(vram_addr >> 2u) & 0x07u);
}

} // namespace

namespace n_e_s::core {

Ppu::Ppu(PpuRegisters *registers, IMmu *mmu)
        : registers_(registers), mmu_(mmu) {}

uint8_t Ppu::read_byte(uint16_t addr) {
    uint8_t byte = open_bus_;

    if (addr == kPpuCtrl || addr == kPpuMask || addr == kOamAddr ||
            addr == kPpuScroll || addr == kPpuAddr) {
        // Return open bus value
    } else if (addr == kPpuStatus) {
        // Bit 0-4 contains whatever is on the bus.
        byte &= 0b0001'1111u;
        byte |= registers_->status.value() & 0b1110'0000u;
        registers_->write_toggle = false;
        open_bus_ = byte;
        clear_vblank_flag();
    } else if (addr == kOamData) {
        byte = oam_data_[registers_->oamaddr];
        open_bus_ = byte;
    } else if (addr == kPpuData) {
        byte = mmu_->read_byte(registers_->vram_addr.value());
        if (registers_->vram_addr.value() < kFirstPaletteData) {
            const uint8_t tmp_buffer = read_buffer_;
            read_buffer_ = byte;
            byte = tmp_buffer;
        } else {
            read_buffer_ = mmu_->read_byte(registers_->vram_addr.value() -
                                           static_cast<uint16_t>(0x1000));
            byte &= 0b0011'1111u;
            byte |= open_bus_ & 0b1100'0000u;
            // Bit 6-7 contains whatever is on the bus.
        }
        open_bus_ = byte;
        increment_vram_address();
    } else {
        byte = mmu_->read_byte(addr);
    }

    return byte;
}

void Ppu::write_byte(uint16_t addr, uint8_t byte) {
    if (addr == kPpuCtrl) {
        open_bus_ = byte;
        const auto new_ctrl = PpuCtrl(byte);
        // Trigger nmi if the nmi-enabled flag goes from 0 to 1 during vblank.
        if (!registers_->ctrl.is_set(7u) && new_ctrl.is_set(7u) &&
                registers_->status.is_set(7u)) {
            on_nmi_();
        }

        registers_->ctrl = new_ctrl;
        registers_->temp_vram_addr.set_nametable(byte);
    } else if (addr == kPpuMask) {
        open_bus_ = byte;
        registers_->mask = PpuMask(byte);
    } else if (addr == kOamAddr) {
        open_bus_ = byte;
        registers_->oamaddr = byte;
    } else if (addr == kPpuStatus) {
        open_bus_ = byte;
    } else if (addr == kOamData) {
        open_bus_ = byte;
        if (!is_rendering_active()) {
            oam_data_[registers_->oamaddr++] = byte;
        }
    } else if (addr == kPpuScroll) {
        open_bus_ = byte;
        if (registers_->write_toggle) { // Second write, Y scroll
            const uint16_t y_scroll = (byte >> 3u);
            const auto fine_y_scroll = static_cast<uint16_t>(byte & 7u);
            registers_->temp_vram_addr.set_fine_scroll_y(fine_y_scroll);
            registers_->temp_vram_addr.set_coarse_scroll_y(y_scroll);
            registers_->write_toggle = false;
        } else { // First write, X Scroll
            const uint16_t x_scroll = (byte >> 3u);
            registers_->temp_vram_addr.set_coarse_scroll_x(x_scroll);
            registers_->fine_x_scroll = static_cast<uint8_t>(byte & 7u);
            registers_->write_toggle = true;
        }
    } else if (addr == kPpuAddr) {
        open_bus_ = byte;
        if (registers_->write_toggle) { // Second write, lower address byte
            registers_->temp_vram_addr = PpuVram(
                    (registers_->temp_vram_addr.value() & 0xFF00u) | byte);
            registers_->vram_addr = registers_->temp_vram_addr;
            registers_->write_toggle = false;
        } else { // First write, upper address byte
            // Valid addresses are $0000-$3FFF; higher addresses will be
            // mirrored down.
            const uint16_t upper_byte = (byte & 0x3Fu) << 8u;
            const uint16_t lower_byte =
                    registers_->temp_vram_addr.value() & 0x00FFu;
            registers_->temp_vram_addr = PpuVram(upper_byte | lower_byte);
            registers_->write_toggle = true;
        }
    } else if (addr == kPpuData) {
        open_bus_ = byte;
        mmu_->write_byte(registers_->vram_addr.value(), byte);
        increment_vram_address();
    } else {
        mmu_->write_byte(addr, byte);
    }
} // namespace n_e_s::core

std::optional<Pixel> Ppu::execute() {
    std::optional<Pixel> pixel;

    if (is_pre_render_scanline()) {
        execute_pre_render_scanline();
    } else if (is_visible_scanline()) {
        pixel = execute_visible_scanline();
    } else if (is_post_render_scanline()) {
        execute_post_render_scanline();
    } else if (is_vblank_scanline()) {
        execute_vblank_scanline();
    }

    update_counters();
    return pixel;
}

void Ppu::set_nmi_handler(const std::function<void()> &on_nmi) {
    on_nmi_ = on_nmi;
}
uint16_t Ppu::scanline() const {
    return registers_->scanline;
}

uint16_t Ppu::cycle() const {
    return registers_->cycle;
}
uint16_t &Ppu::scanline() {
    return registers_->scanline;
}

uint16_t &Ppu::cycle() {
    return registers_->cycle;
}

void Ppu::update_counters() {
    if (cycle() == kLastCycleInScanline) {
        cycle() = 0;
        if (scanline() == kLastScanlineInFrame) {
            scanline() = 0;
            registers_->odd_frame = !registers_->odd_frame;
            if (registers_->odd_frame &&
                    registers_->mask.is_rendering_enabled()) {
                cycle() = 1;
            }
        } else {
            ++scanline();
        }
    } else {
        ++cycle();
    }
}

bool Ppu::is_pre_render_scanline() const {
    return scanline() == kPreRenderScanline;
}

bool Ppu::is_visible_scanline() const {
    // Visible scanlines starts at 0
    return scanline() <= kVisibleScanlineEnd;
}

bool Ppu::is_post_render_scanline() const {
    return scanline() == kPostRenderScanline;
}

bool Ppu::is_vblank_scanline() const {
    return scanline() >= kVBlankScanlineStart &&
           scanline() <= kVBlankScanlineEnd;
}

bool Ppu::is_rendering_active() const {
    return registers_->mask.is_rendering_enabled() &&
           (is_pre_render_scanline() || is_visible_scanline());
}

uint8_t Ppu::get_vram_address_increment() const {
    uint8_t addr_increment = 1u;

    if (registers_->ctrl.is_set(2u)) {
        addr_increment = 32u;
    }

    return addr_increment;
}

void Ppu::increment_vram_address() {
    registers_->vram_addr = PpuVram(
            registers_->vram_addr.value() + get_vram_address_increment());
}

void Ppu::execute_pre_render_scanline() {
    fetch();
    increase_scroll_counters();
    if (cycle() == 1) {
        clear_vblank_flag();
    }
}

std::optional<Pixel> Ppu::execute_visible_scanline() {
    fetch();
    increase_scroll_counters();
    return pixel();
}

void Ppu::execute_post_render_scanline() {
    // The ppu is idle this scanline
}

void Ppu::execute_vblank_scanline() {
    if (scanline() == 241 && cycle() == 1) {
        set_vblank_flag();
        if (registers_->ctrl.is_set(7u)) {
            on_nmi_();
        }
    }
}

void Ppu::set_vblank_flag() {
    registers_->status.set_bit(7u);
}

void Ppu::clear_vblank_flag() {
    registers_->status.clear_bit(7u);
}

void Ppu::shift_registers() {
    registers_->pattern_table_shifter_low <<= 1u;
    registers_->pattern_table_shifter_hi <<= 1u;

    registers_->attribute_table_shifter_low <<= 1u;
    registers_->attribute_table_shifter_hi <<= 1u;
}

void Ppu::increase_scroll_counters() {
    if (!registers_->mask.is_rendering_enabled()) {
        return;
    }

    // During pixels 280 through 304 of this scanline, the vertical scroll bits
    // are reloaded if rendering is enabled. vert(v) == vert(t)
    if (scanline() == kPreRenderScanline) {
        if (cycle() >= 280 && cycle() <= 304) {
            auto addr = registers_->vram_addr.value();
            addr &= static_cast<uint16_t>(~0x7BE0u);
            addr |= (registers_->temp_vram_addr.value() & 0x7BE0u);
            registers_->vram_addr = PpuVram(addr);
        }
    }

    const bool should_increase_coarse_x =
            !(cycle() == 0 || (cycle() >= 256 && cycle() <= 320));
    if (should_increase_coarse_x && (cycle() % 8) == 0) {
        registers_->vram_addr.increase_coarse_x();
    }

    if (cycle() == 256) {
        registers_->vram_addr.increase_y();
    } else if (cycle() == 257) {
        // Copies all bits related to horizontal position from
        // temporal to current vram_address
        // From http://wiki.nesdev.com/w/index.php/PPU_scrolling
        // If rendering is enabled, the PPU copies all bits related to
        // horizontal position from t to v:
        auto addr = registers_->vram_addr.value();
        addr &= ~0x41Fu;
        addr |= registers_->temp_vram_addr.value() & 0x41Fu;
        registers_->vram_addr = PpuVram(addr);
    }
}
void Ppu::fetch() {
    if ((cycle() >= 1 && cycle() <= 256) ||
            (cycle() >= 321 && cycle() <= 336)) {
        shift_registers();
        const uint16_t background_pattern_table_base_address =
                registers_->ctrl.is_set(4u) ? 0x1000u : 0x0000u;
        const uint8_t fine_scroll_y = registers_->vram_addr.fine_scroll_y();

        switch ((cycle() - 1) % 8) {
        case 0: {
            registers_->pattern_table_shifter_low =
                    (registers_->pattern_table_shifter_low & 0xFF00u) |
                    registers_->pattern_table_latch_low;
            registers_->pattern_table_shifter_hi =
                    (registers_->pattern_table_shifter_hi & 0xFF00u) |
                    registers_->pattern_table_latch_hi;

            registers_->attribute_table_shifter_low =
                    (registers_->attribute_table_shifter_low & 0xFF00u) |
                    ((registers_->attribute_table_latch & 0b01u) ? 0xFFu
                                                                 : 0x00u);
            registers_->attribute_table_shifter_hi =
                    (registers_->attribute_table_shifter_hi & 0xFF00u) |
                    ((registers_->attribute_table_latch & 0b10u) ? 0xFFu
                                                                 : 0x00u);

            registers_->name_table = registers_->name_table_latch;
            const uint16_t nametable_address =
                    get_nametable_address(registers_->vram_addr.value());
            registers_->name_table_latch = mmu_->read_byte(nametable_address);
            break;
        }
        case 2: {
            const uint16_t attribute_address =
                    get_attribute_address(registers_->vram_addr.value());
            uint8_t byte = mmu_->read_byte(attribute_address);
            // Figure out which quadrant we are in and get the two corresponding
            // bits.
            // 7654 3210
            // |||| ||++- Color bits 3-2 for top left quadrant of this byte
            // |||| ++--- Color bits 3-2 for top right quadrant of this byte
            // ||++------ Color bits 3-2 for bottom left quadrant of this byte
            // ++-------- Color bits 3-2 for bottom right quadrant of this byte
            const uint8_t coarse_y = registers_->vram_addr.coarse_scroll_y();
            const uint8_t coarse_x = registers_->vram_addr.coarse_scroll_x();
            if (coarse_y % 4u >= 2u) {
                // We are in the bottom quadrant
                byte >>= 4u;
            }
            if (coarse_x % 4u >= 2u) {
                // We are in the right quadrant
                byte >>= 2u;
            }
            byte &= 0x0000'0003u;
            registers_->attribute_table_latch = byte;
            break;
        }
        case 4: {
            const uint16_t index = background_pattern_table_base_address +
                                   (registers_->name_table_latch * 16u) +
                                   fine_scroll_y;
            registers_->pattern_table_latch_low = mmu_->read_byte(index);
            break;
        }
        case 6: {
            const uint16_t index = background_pattern_table_base_address +
                                   (registers_->name_table_latch * 16u) +
                                   fine_scroll_y;
            // +8 for next bit plane
            registers_->pattern_table_latch_hi = mmu_->read_byte(index + 0x8u);
            break;
        }
        };
    } else if (cycle() == 337 || cycle() == 339) {
        // Unused fetches. Uses the same nametable address as will be fetched
        // at the beginning of the next scanline (tile 3 since tile 1 and 2
        // were fetched already in the end of this scanline).
        const uint16_t nametable_address =
                get_nametable_address(registers_->vram_addr.value());
        std::ignore = mmu_->read_byte(nametable_address);
    }
}

std::optional<Pixel> Ppu::pixel() {
    const bool visible_cycle = cycle() >= 1u && cycle() <= 256u;
    if (!visible_cycle) {
        return std::nullopt;
    }

    uint8_t background_pixel = 0u;
    uint8_t background_palette = 0u;

    // The first 8 pixels should be skipped if "render background left" is
    // not set
    const bool is_background_visible =
            registers_->mask.render_background() &&
            (registers_->mask.render_background_left() || cycle() > 8u);

    if (is_background_visible) {
        const auto fine_x = registers_->fine_x_scroll;
        const uint16_t horizontal_pixel_index = 0x8000u >> fine_x;

        const uint8_t low_pixel = (registers_->pattern_table_shifter_low &
                                          horizontal_pixel_index) > 0u;
        const uint8_t high_pixel = (registers_->pattern_table_shifter_hi &
                                           horizontal_pixel_index) > 0u;
        background_pixel = static_cast<uint8_t>(high_pixel << 1u) | low_pixel;

        const uint8_t low_attribute = (registers_->attribute_table_shifter_low &
                                              horizontal_pixel_index) > 0u;
        const uint8_t high_attribute = (registers_->attribute_table_shifter_hi &
                                               horizontal_pixel_index) > 0u;
        background_palette =
                static_cast<uint8_t>(high_attribute << 1u) | low_attribute;
    }
    // TODO(JN): render sprites and implement sprite priority.
    uint8_t combined_pixel = background_pixel;
    uint8_t combined_palette = background_palette;

    if (background_pixel == 0u) {
        // Both are transparent, output background ($3F00).
        combined_pixel = 0u;
        combined_palette = 0u;
    }

    // Each palette is 4 bytes and starts at 0x3F00.
    const uint16_t color_address =
            0x3F00u + combined_palette * 4u + combined_pixel;
    // TODO(JN): Handle greyscale.
    const uint8_t palette_index = mmu_->read_byte(color_address);
    const auto color = get_color_from_palette_index(palette_index);

    const auto x = static_cast<uint8_t>(cycle() - 1u);
    const auto y = static_cast<uint8_t>(scanline());
    return Pixel{.x = x, .y = y, .color = color};
}

Color Ppu::get_color_from_palette_index(uint8_t index) const {
    return kPalette[index % kPalette.size()];
}

} // namespace n_e_s::core
