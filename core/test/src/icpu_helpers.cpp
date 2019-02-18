#include "icpu_helpers.h"

#include "hexprinter.h"

#include <ostream>

using namespace n_e_s::core::test;

namespace n_e_s::core {

bool operator==(const ICpu::Registers &a, const ICpu::Registers &b) {
    return a.pc == b.pc && a.sp == b.sp && a.a == b.a && a.x == b.x &&
           a.y == b.y && a.p == b.p;
}

void PrintTo(const ICpu::Registers &r, std::ostream *os) {
    *os << "PC: " << hex_out_s(r.pc);
    *os << " SP: " << hex_out_s(r.sp);
    *os << " A: " << hex_out_s(r.a);
    *os << " X: " << hex_out_s(r.x);
    *os << " Y: " << hex_out_s(r.y);
    *os << " P: ";
    *os << (r.p & N_FLAG ? "N" : "-");
    *os << (r.p & V_FLAG ? "V" : "-");
    *os << "-";
    *os << (r.p & B_FLAG ? "B" : "-");
    *os << (r.p & D_FLAG ? "D" : "-");
    *os << (r.p & I_FLAG ? "I" : "-");
    *os << (r.p & Z_FLAG ? "Z" : "-");
    *os << (r.p & C_FLAG ? "C" : "-");
    *os << std::endl;
}

} // namespace n_e_s::core
