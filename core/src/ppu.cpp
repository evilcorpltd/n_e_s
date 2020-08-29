#include "ppu.h"

#include "nes/core/invalid_address.h"

namespace {

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

} // namespace

namespace n_e_s::core {

Ppu::Ppu(PpuRegisters *registers, IMmu *mmu)
        : registers_(registers), mmu_(mmu) {}

uint8_t Ppu::read_byte(uint16_t addr) {
    uint8_t byte = 0;

    if (addr == kPpuStatus) {
        byte = registers_->status;
        clear_vblank_flag();
    } else if (addr == kOamData) {
        byte = oam_data_[registers_->oamaddr];
    } else if (addr == kPpuData) {
        byte = mmu_->read_byte(registers_->vram_addr);
        if (registers_->vram_addr < kFirstPaletteData) {
            const uint8_t tmp_buffer = read_buffer_;
            read_buffer_ = byte;
            byte = tmp_buffer;
        } else {
            read_buffer_ = mmu_->read_byte(
                    registers_->vram_addr - static_cast<uint16_t>(0x1000));
        }
        increment_vram_address();
    } else {
        byte = mmu_->read_byte(addr);
    }

    return byte;
}

void Ppu::write_byte(uint16_t addr, uint8_t byte) {
    if (addr == kPpuCtrl) {
        // When we have implemented NMI we should check if NMI is set to be
        // enabled (bit 7). If this is the case and we currently are in
        // vertical blanking a NMI shall be generated.
        registers_->ctrl = byte;
        auto name_table_bits = static_cast<uint16_t>(byte & 3u);
        registers_->temp_vram_addr &=
                static_cast<uint16_t>(0b1111'0011'1111'1111);
        registers_->temp_vram_addr |=
                static_cast<uint16_t>(name_table_bits << 10u);
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
            uint16_t y_scroll = (byte >> 3u);
            auto fine_y_scroll = static_cast<uint16_t>(byte & 7u);
            registers_->temp_vram_addr &=
                    static_cast<uint16_t>(0b1000'1100'0001'1111);
            registers_->temp_vram_addr |= static_cast<uint16_t>(y_scroll << 5u);
            registers_->temp_vram_addr |=
                    static_cast<uint16_t>(fine_y_scroll << 12u);
            registers_->write_toggle = false;
        } else { // First write, X Scroll
            uint16_t x_scroll = (byte >> 3u);
            registers_->temp_vram_addr &=
                    static_cast<uint16_t>(0b1111'1111'1110'0000);
            registers_->temp_vram_addr |= x_scroll;
            registers_->fine_x_scroll = static_cast<uint8_t>(byte & 7u);
            registers_->write_toggle = true;
        }
    } else if (addr == kPpuAddr) {
        if (registers_->write_toggle) { // Second write, lower address byte
            registers_->temp_vram_addr |= byte;
            registers_->vram_addr = registers_->temp_vram_addr;
            registers_->write_toggle = false;
        } else { // First write, upper address byte
            uint16_t upper_addr_byte = byte;
            registers_->temp_vram_addr = (upper_addr_byte << 8u);
            registers_->write_toggle = true;
        }
    } else if (addr == kPpuData) {
        mmu_->write_byte(registers_->vram_addr, byte);
        increment_vram_address();
    } else {
        mmu_->write_byte(addr, byte);
    }
} // namespace n_e_s::core

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

void Ppu::set_nmi_handler(const std::function<void()> &on_nmi) {
    on_nmi_ = on_nmi;
}
uint16_t Ppu::scanline() const {
    return scanline_;
}

uint16_t Ppu::cycle() const {
    return cycle_;
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
    return (registers_->mask & (1u << 3u)) || (registers_->mask & (1u << 4u));
}

bool Ppu::is_rendering_active() const {
    return is_rendering_enabled() &&
           (is_pre_render_scanline() || is_visible_scanline());
}

uint8_t Ppu::get_vram_address_increment() const {
    uint8_t addr_increment = 1;

    if (registers_->ctrl & (1u << 2u)) {
        addr_increment = 32;
    }

    return addr_increment;
}

void Ppu::increment_vram_address() {
    registers_->vram_addr += get_vram_address_increment();
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
        if (registers_->ctrl & (1u << 7u)) {
            on_nmi_();
        }
    }
}

void Ppu::set_vblank_flag() {
    registers_->status |= (1u << 7u);
}

void Ppu::clear_vblank_flag() {
    registers_->status &= ~(1u << 7u);
}

} // namespace n_e_s::core
