#include "ippu_helpers.h"

#include <fmt/format.h>
#include <ostream>

namespace n_e_s::core {

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const PpuRegisters &r, std::ostream *os) {
    *os << fmt::format(
            "Cycle: {} Scanline: {} Ctrl: {:#04x} ScrollX: {:#04x} Mask: "
            "{:#04x} OamAddr: {:#04x} "
            "Status: {:#04x} VramAddr: {:#06x} TmpVramAddr: {:#06x} "
            "WriteToggle: {} OddFrame: {} NameTbl: {:#04x}[{:#04x}] "
            "PatternTblHi: {:#06x}[{:#04x}] "
            "PatternTblLow: {:#06x}[{:#04x}] "
            "Attr: {:#06x}:{:#06x}[{:#04x}]\n",
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
            r.odd_frame,
            r.name_table,
            r.name_table_latch,
            r.pattern_table_shifter_hi,
            r.pattern_table_latch_hi,
            r.pattern_table_shifter_low,
            r.pattern_table_latch_low,
            r.attribute_table_shifter_low,
            r.attribute_table_shifter_hi,
            r.attribute_table_latch);
}

} // namespace n_e_s::core
