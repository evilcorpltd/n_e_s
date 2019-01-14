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

    // Object Atribute Memory
    constexpr static uint16_t OAM_SIZE{256};
    uint8_t oam_data_[OAM_SIZE]{};

    // PPU vram. The size of this memory bank shall only be 2kB, but for now
    // we let it fill the whole address space which the PPU has. This should be
    // changed once we have support for letting the cartridge map stuff in this
    // address space.
    constexpr static uint16_t VRAM_SIZE{6 * 1024};
    uint8_t ppu_vram_[VRAM_SIZE]{};

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

    // Gets how much the vram address should be incremented after read or write
    // has been done. Returns 1 if bit 2 in the ctrl register is not set and 32
    // if the bit is set.
    uint8_t get_vram_address_increment() const;

    void execute_pre_render_scanline();
    void execute_visible_scanline();
    void execute_post_render_scanline();
    void execute_vblank_scanline();

    void set_vblank_flag();
    void clear_vblank_flag();
};

} // namespace n_e_s::core
