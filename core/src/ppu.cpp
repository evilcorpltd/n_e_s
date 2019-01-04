#include "ppu.h"

#include "core/invalid_address.h"

namespace {

const uint16_t kLastCycleInScanline = 340;
const uint16_t kLastScanlineInFrame = 261;
const uint16_t kPreRenderScanline = 261;
const uint16_t kVisibleScanlineEnd = 239;
const uint16_t kPostRenderScanline = 240;
const uint16_t kVBlankScanlineStart = 241;
const uint16_t kVBlankScanlineEnd = 260;

} // namespace

namespace n_e_s::core {

Ppu::Ppu(IPpu::Registers *registers)
        : registers_(registers), scanline_(0), cycle_(0) {}

uint8_t Ppu::read_byte(uint16_t addr) {
    if (addr == 0x2002) {
        const uint8_t status = registers_->status;
        clear_vblank_flag();
        return status;
    }

    throw InvalidAddress(addr);
}

void Ppu::write_byte(uint16_t addr, uint8_t) {
    throw InvalidAddress(addr);
}

void Ppu::execute() {
    if (is_pre_render_scanline()) {
        execute_pre_render_scanline();
    } else if (is_visible_scanline()) {
        execute_visible_scanline();
    } else if (is_post_render_scanline()) {
        execute_post_render_scanline();
    } else if (is_vblank_scanline()) {
        execute_vblank_scanline();
    }

    update_counters();
}

void Ppu::update_counters() {
    if (cycle_ == kLastCycleInScanline) {
        cycle_ = 0;
        if (scanline_ == kLastScanlineInFrame) {
            scanline_ = 0;
        } else {
            ++scanline_;
        }
    } else {
        ++cycle_;
    }
}

bool Ppu::is_pre_render_scanline() const {
    return scanline_ == kPreRenderScanline;
}

bool Ppu::is_visible_scanline() const {
    // Visible scanlines starts at 0
    return scanline_ <= kVisibleScanlineEnd;
}

bool Ppu::is_post_render_scanline() const {
    return scanline_ == kPostRenderScanline;
}

bool Ppu::is_vblank_scanline() const {
    return scanline_ >= kVBlankScanlineStart && scanline_ <= kVBlankScanlineEnd;
}

void Ppu::execute_pre_render_scanline() {
    if (cycle_ == 1) {
        clear_vblank_flag();
    }
}

void Ppu::execute_visible_scanline() {}

void Ppu::execute_post_render_scanline() {}

void Ppu::execute_vblank_scanline() {
    if (cycle_ == 1) {
        set_vblank_flag();
    }
}

void Ppu::set_vblank_flag() {
    registers_->status |= (1 << 7);
}

void Ppu::clear_vblank_flag() {
    registers_->status &= ~(1 << 7);
}

} // namespace n_e_s::core
