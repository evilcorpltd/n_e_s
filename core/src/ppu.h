#pragma once

#include "core/ippu.h"

#include <memory>

namespace n_e_s::core {

class Ppu : public IPpu {
public:
    Ppu(IPpu::Registers *registers);

    uint8_t read_byte(uint16_t addr) override;
    void write_byte(uint16_t addr, uint8_t byte) override;

    void execute() override;

private:
    IPpu::Registers *const registers_;
    uint16_t scanline_;
    uint16_t cycle_;

    // Updates cycle and scanline counters
    void update_counters();

    bool is_pre_render_scanline() const;
    bool is_visible_scanline() const;
    bool is_post_render_scanline() const;
    bool is_vblank_scanline() const;

    // Rendering is enabled if bit 3 or bit 4 is enabled in the mask register.
    bool is_rendering_enabled() const;

    // Rendering is active if rendering is enabled and the PPU is currently
    // in pre render scanline or a visible scanline.
    bool is_rendering_active() const;

    void execute_pre_render_scanline();
    void execute_visible_scanline();
    void execute_post_render_scanline();
    void execute_vblank_scanline();

    void set_vblank_flag();
    void clear_vblank_flag();
};

} // namespace n_e_s::core
