#include "icpu_helpers.h"

#include <fmt/format.h>
#include <ostream>

namespace n_e_s::core {

bool operator==(const CpuRegisters &a, const CpuRegisters &b) {
    return a.pc == b.pc && a.sp == b.sp && a.a == b.a && a.x == b.x &&
           a.y == b.y && a.p == b.p;
}

// Required by gtest to use pascal case.
// NOLINTNEXTLINE(readability-identifier-naming)
void PrintTo(const CpuRegisters &r, std::ostream *os) {
    *os << fmt::format(
            "PC: {:#06x} SP: {:#04x} A: {:#04x} x: {:#04x} Y: {:#04x}",
            r.pc,
            r.sp,
            r.a,
            r.x,
            r.y);
    *os << fmt::format(" P: {}{}{}{}{}{}{}{}",
            r.p & N_FLAG ? "N" : "-",
            r.p & V_FLAG ? "V" : "-",
            r.p & FLAG_5 ? "5" : "-",
            r.p & B_FLAG ? "B" : "-",
            r.p & D_FLAG ? "D" : "-",
            r.p & I_FLAG ? "I" : "-",
            r.p & Z_FLAG ? "Z" : "-",
            r.p & C_FLAG ? "C" : "-");
    *os << std::endl;
}

} // namespace n_e_s::core
