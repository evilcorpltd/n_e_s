#include "ippu_helpers.h"

#include <fmt/format.h>
#include <ostream>

namespace n_e_s::core {

bool operator==(const PpuRegisters &a, const PpuRegisters &b) {
    return a.scanline == b.scanline && a.cycle == b.cycle && a.ctrl == b.ctrl &&
           a.mask == b.mask && a.status == b.status && a.oamaddr == b.oamaddr &&
           a.fine_x_scroll == b.fine_x_scroll && a.vram_addr == b.vram_addr &&
           a.temp_vram_addr == b.temp_vram_addr &&
           a.write_toggle == b.write_toggle && a.odd_frame == b.odd_frame;
}

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const PpuRegisters &r, std::ostream *os) {
    *os << fmt::format(
            "Cycle: {} Scanline: {} Ctrl: {:#04x} ScrollX: {:#04x} Mask: "
            "{:#04x} OamAddr: {:#04x} "
            "Status: {:#04x} VramAddr: {:#06x} TmpVramAddr: {:#06x} "
            "WriteToggle: {} OddFrame: {}\n",
            r.cycle,
            r.scanline,
            r.ctrl,
            r.fine_x_scroll,
            r.mask.value(),
            r.oamaddr,
            r.status,
            r.vram_addr.value(),
            r.temp_vram_addr.value(),
            r.write_toggle,
            r.odd_frame);
}

} // namespace n_e_s::core
