#pragma once

#include "nes/core/immu.h"
#include "nes/core/ippu.h"

#include <memory>

namespace n_e_s::core {

class Ppu final : public IPpu {
public:
    explicit Ppu(PpuRegisters *registers, IMmu *mmu);

    uint8_t read_byte(uint16_t addr) override;
    void write_byte(uint16_t addr, uint8_t byte) override;

    void execute() override;

    void set_nmi_handler(const std::function<void()> &on_nmi) override;

    uint16_t scanline() const override;
    uint16_t cycle() const override;

private:
    PpuRegisters *const registers_;
    IMmu *const mmu_;

    std::function<void()> on_nmi_{[] {}};

    uint16_t scanline_{0};
    uint16_t cycle_{0};
    uint8_t read_buffer_{0};

    // Object Atribute Memory
    constexpr static uint16_t kOamSize{256};
    uint8_t oam_data_[kOamSize]{};

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

    // Increments the VRAM address. Shall be done after reading or writing
    // from/to the VRAM.
    void increment_vram_address();

    void execute_pre_render_scanline();
    static void execute_visible_scanline();
    void execute_post_render_scanline();
    void execute_vblank_scanline();

    void set_vblank_flag();
    void clear_vblank_flag();
};

} // namespace n_e_s::core
