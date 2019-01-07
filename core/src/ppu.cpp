#include "ppu.h"

#include "core/invalid_address.h"

namespace {

const uint16_t kPpuCtrl = 0x2000;
const uint16_t kPpuMask = 0x2001;
const uint16_t kPpuStatus = 0x2002;
const uint16_t kOamAddr = 0x2003;
const uint16_t kOamData = 0x2004;
const uint16_t kPpuScroll = 0x2005;
const uint16_t kPpuAddr = 0x2006;

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
    if (addr == kPpuStatus) {
        const uint8_t status = registers_->status;
        clear_vblank_flag();
        return status;
    }

    throw InvalidAddress(addr);
}

void Ppu::write_byte(uint16_t addr, uint8_t byte) {
    if (addr == kPpuCtrl) {
        // When we have implemented NMI we should check if NMI is set to be
        // enabled (bit 7). If this is the case and we currently are in
        // vertical blanking a NMI shall be generated.
        registers_->ctrl = byte;
        uint16_t name_table_bits = (byte & 3);
        registers_->temp_vram_addr &= 0b1111'0011'1111'1111;
        registers_->temp_vram_addr |= (name_table_bits << 10);
    } else if (addr == kPpuMask) {
        registers_->mask = byte;
    } else if (addr == kOamAddr) {
        registers_->oamaddr = byte;
    } else if (addr == kOamData) {
        if (!is_rendering_active()) {
            oam_data_[registers_->oamaddr++] = byte;
        }
    } else if (addr == kPpuScroll) {
        if (registers_->write_toggle) { // Second write, Y scroll
            uint16_t y_scroll = (byte >> 3);
            uint16_t fine_y_scroll = (byte & 7);
            registers_->temp_vram_addr &= 0b1000'1100'0001'1111;
            registers_->temp_vram_addr |= (y_scroll << 5);
            registers_->temp_vram_addr |= (fine_y_scroll << 12);
            registers_->write_toggle = false;
        } else { // First write, X Scroll
            uint16_t x_scroll = (byte >> 3);
            registers_->temp_vram_addr &= 0b1111'1111'1110'0000;
            registers_->temp_vram_addr |= x_scroll;
            registers_->fine_x_scroll = (byte & 7);
            registers_->write_toggle = true;
        }
    } else if (addr == kPpuAddr) {
        if (registers_->write_toggle) { // Second write, lower address byte
            registers_->temp_vram_addr |= byte;
            registers_->vram_addr = registers_->temp_vram_addr;
            registers_->write_toggle = false;
        } else { // First write, upper address byte
            uint16_t upper_addr_byte = byte;
            registers_->temp_vram_addr = (upper_addr_byte << 8);
            registers_->write_toggle = true;
        }
    } else {
        throw InvalidAddress(addr);
    }
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

bool Ppu::is_rendering_enabled() const {
    return (registers_->mask & (1 << 3)) || (registers_->mask & (1 << 4));
}

bool Ppu::is_rendering_active() const {
    return is_rendering_enabled() &&
           (is_pre_render_scanline() || is_visible_scanline());
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
