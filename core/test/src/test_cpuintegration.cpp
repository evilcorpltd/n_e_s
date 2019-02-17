#include "core/cpu_factory.h"

#include "fake_mmu.h"
#include "hexprinter.h"
#include "mock_mmu.h"

#include <gtest/gtest.h>
#include <ostream>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

namespace n_e_s::core {

static bool operator==(const ICpu::Registers &a, const ICpu::Registers &b) {
    return a.pc == b.pc && a.sp == b.sp && a.a == b.a && a.x == b.x &&
           a.y == b.y && a.p == b.p;
}

static void PrintTo(const ICpu::Registers &r, std::ostream *os) {
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

namespace {

const uint16_t kStackOffset = 0x0100;
const uint16_t kResetAddress = 0xFFFC;
const uint16_t kBrkAddress = 0xFFFE;

class CpuIntegrationTest : public ::testing::Test {
public:
    CpuIntegrationTest()
            : registers(),
              mmu(),
              cpu{CpuFactory::create(&registers, &mmu)},
              expected() {}

    void step_execution(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            cpu->execute();
        }
    }

    int run_until_brk(int max_cycles = 10000) {
        cpu->reset();

        for (int cycles = 1; cycles < max_cycles; ++cycles) {
            step_execution(1);

            if (registers.pc == mmu.read_word(kBrkAddress)) {
                return cycles;
            }
        }
        return max_cycles;
    }

    void load_hex_dump(uint16_t address, std::vector<uint8_t> data) {
        for (auto d : data) {
            mmu.write_byte(address++, d);
        }
    }

    void set_reset_address(uint16_t address) {
        mmu.write_word(kResetAddress, address);
    }

    void set_break_address(uint16_t address) {
        mmu.write_word(kBrkAddress, address);
    }

    ICpu::Registers registers;
    FakeMmu mmu;
    std::unique_ptr<ICpu> cpu;

    ICpu::Registers expected;
};

TEST_F(CpuIntegrationTest, simple_program) {
    // Address  Hexdump   Dissassembly
    // -------------------------------
    // $0600    a9 01     LDA #$01
    // $0602    8d 00 04  STA $0400
    // $0605    a2 05     LDX #$05
    // $0607    8e 01 04  STX $0401
    // $060a    a0 08     LDY #$08
    // $060c    8c 02 04  STY $0402
    // $060f    00        BRK
    load_hex_dump(0x0600,
            {0xa9,
                    0x01,
                    0x8d,
                    0x00,
                    0x04,
                    0xa2,
                    0x05,
                    0x8e,
                    0x01,
                    0x04,
                    0xa0,
                    0x08,
                    0x8c,
                    0x02,
                    0x04,
                    0x00});
    set_reset_address(0x0600);
    set_break_address(0xDEAD);

    expected.a = 0x01;
    expected.x = 0x05;
    expected.y = 0x08;
    expected.sp = kStackOffset - 3;
    expected.pc = 0xDEAD;

    const int expected_cycles = 2 + 4 + 2 + 4 + 2 + 4 + 7;

    EXPECT_EQ(expected_cycles, run_until_brk());
    EXPECT_EQ(expected, registers);
    EXPECT_EQ(0x01, mmu.read_byte(0x0400));
    EXPECT_EQ(0x05, mmu.read_byte(0x0401));
    EXPECT_EQ(0x08, mmu.read_byte(0x0402));
}

TEST_F(CpuIntegrationTest, branch) {
    // Address  Hexdump   Dissassembly
    // -------------------------------
    // $0600    a2 08     LDX #$08
    // $0602    ca        DEX
    // $0603    8e 00 02  STX $0200
    // $0606    e0 03     CPX #$03
    // $0608    d0 f8     BNE $0602
    // $060a    8e 01 02  STX $0201
    // $060d    00        BRK
    load_hex_dump(0x0600,
            {0xa2,
                    0x08,
                    0xca,
                    0x8e,
                    0x00,
                    0x02,
                    0xe0,
                    0x03,
                    0xd0,
                    0xf8,
                    0x8e,
                    0x01,
                    0x02,
                    0x00});

    set_reset_address(0x0600);
    set_break_address(0xDEAD);

    expected.a = 0x00;
    expected.x = 0x03;
    expected.y = 0x00;
    expected.sp = kStackOffset - 3;
    expected.pc = 0xDEAD;
    expected.p = C_FLAG | Z_FLAG;

    // BNE takes 3 cycles when branch is taken, two when it is not.
    const int expected_cycles =
            2 + (2 + 4 + 2 + 3) * 4 + (2 + 4 + 2 + 2) + 4 + 7;

    EXPECT_EQ(expected_cycles, run_until_brk());
    EXPECT_EQ(expected, registers);
}

} // namespace
