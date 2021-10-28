#include "ippu_helpers.h"

#include <fmt/format.h>
#include <ostream>

namespace n_e_s::core {

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const PpuRegisters &r, std::ostream *os) {
    *os << fmt::format(
            "Ctrl: {:#04x} ScrollX: {:#04x} Mask: {:#04x} OamAddr: {:#04x} "
            "Status: {:#04x} VramAddr: {:#06x} TmpVramAddr: {:#06x} "
            "WriteToggle: {}\n",
            r.ctrl,
            r.fine_x_scroll,
            r.mask,
            r.oamaddr,
            r.status,
            r.vram_addr,
            r.temp_vram_addr,
            r.write_toggle);
}

} // namespace n_e_s::core
