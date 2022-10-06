#include "nes/core/cpu_factory.h"

#include "icpu_helpers.h"
#include "nes/core/test/fake_mmu.h"
#include "nes/core/test/fake_ppu.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;
using namespace n_e_s::core::test;

namespace {

const uint16_t kStackOffset = 0x0100;
const uint16_t kNmiAddress = 0xFFFA;
const uint16_t kResetAddress = 0xFFFC;
const uint16_t kBrkAddress = 0xFFFE;

class CpuIntegrationTest : public ::testing::Test {
public:
    CpuIntegrationTest() : registers(), mmu(), ppu(), expected() {
        registers.sp = expected.sp = 0xFF;
    }

    void step_execution(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            cpu->execute();
        }
    }

    int run_until_brk(int max_cycles = 10000) {
        cpu->reset();

        for (int cycles = 1; cycles < max_cycles; ++cycles) {
            step_execution(1);

            const uint16_t break_pc =
                    mmu.read_byte(kBrkAddress) |
                    static_cast<uint16_t>(
                            mmu.read_byte(kBrkAddress + 1u) << 8u);
            if (registers.pc == break_pc) {
                return cycles;
            }
        }
        return max_cycles;
    }

    void load_hex_dump(uint16_t address, const std::vector<uint8_t> &data) {
        for (auto d : data) {
            mmu.write_byte(address++, d);
        }
    }

    void set_reset_address(uint16_t address) {
        write_word(kResetAddress, address);
    }

    void set_break_address(uint16_t address) {
        write_word(kBrkAddress, address);
    }

    void set_nmi_address(uint16_t address) {
        write_word(kNmiAddress, address);
    }

    void write_word(uint16_t address, uint16_t value) {
        const auto low = static_cast<uint8_t>(value & 0xFFu);
        const auto upper = static_cast<uint8_t>(value >> 8u);
        mmu.write_byte(address, low);
        mmu.write_byte(address + 1u, upper);
    }

    CpuRegisters registers;
    FakeMmu mmu;
    FakePpu ppu;
    std::unique_ptr<IMos6502> cpu{
            CpuFactory::create_mos6502(&registers, &mmu, &ppu)};

    CpuRegisters expected;
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
                    0x00,
                    0x00}); // Needed for previous BRK dummy read
    set_reset_address(0x0600);
    set_break_address(0xDEAD);

    expected.a = 0x01;
    expected.x = 0x05;
    expected.y = 0x08;
    expected.sp =
            registers.sp - static_cast<uint8_t>(3); // PC (2 bytes) + P (1 byte)
    expected.pc = 0xDEAD;
    expected.p = I_FLAG;

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
                    0x00,
                    0x00}); // Needed for previous BRK dummy read

    set_reset_address(0x0600);
    set_break_address(0xDEAD);

    expected.a = 0x00;
    expected.x = 0x03;
    expected.y = 0x00;
    expected.sp =
            registers.sp - static_cast<uint8_t>(3); // PC (2 bytes) + P (1 byte)
    expected.pc = 0xDEAD;
    expected.p = static_cast<uint8_t>(C_FLAG | Z_FLAG) | I_FLAG;

    // BNE takes 3 cycles when branch is taken, two when it is not.
    const int expected_cycles =
            2 + (2 + 4 + 2 + 3) * 4 + (2 + 4 + 2 + 2) + 4 + 7;

    EXPECT_EQ(expected_cycles, run_until_brk());
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuIntegrationTest, stack) {
    // Address  Hexdump   Dissassembly
    // -------------------------------
    // $0600    a2 00     LDX #$00
    // $0602    a0 00     LDY #$00
    // $0604    8a        TXA
    // $0605    99 00 02  STA $0200,Y
    // $0608    48        PHA
    // $0609    e8        INX
    // $060a    c8        INY
    // $060b    c0 10     CPY #$10
    // $060d    d0 f5     BNE $0604
    // $060f    68        PLA
    // $0610    99 00 02  STA $0200,Y
    // $0613    c8        INY
    // $0614    c0 20     CPY #$20
    // $0616    d0 f7     BNE $060f
    // $0618    00        BRK
    load_hex_dump(0x0600,
            {0xa2,
                    0x00,
                    0xa0,
                    0x00,
                    0x8a,
                    0x99,
                    0x00,
                    0x02,
                    0x48,
                    0xe8,
                    0xc8,
                    0xc0,
                    0x10,
                    0xd0,
                    0xf5,
                    0x68});
    load_hex_dump(0x0610,
            {0x99,
                    0x00,
                    0x02,
                    0xc8,
                    0xc0,
                    0x20,
                    0xd0,
                    0xf7,
                    0x00,
                    0x00,
                    0x00}); // Needed for previous BRK dummy read

    // STA with absolute indexed addressing will first read from the effective
    // address before writing to it, so we need to initialize the fake mmu with
    // some data at those addresses.
    for (uint8_t i = 0; i <= 0x20; ++i) {
        mmu.write_byte(
                static_cast<uint16_t>(0x0200) + i, static_cast<uint8_t>(0xBA));
    }

    set_reset_address(0x0600);
    set_break_address(0xDEAD);

    expected.x = 0x10;
    expected.y = 0x20;
    expected.sp =
            registers.sp - static_cast<uint8_t>(3); // PC (2 bytes) + P (1 byte)
    expected.pc = 0xDEAD;
    expected.p = static_cast<uint8_t>(C_FLAG | Z_FLAG) | I_FLAG;

    const int pre_loop = 2 + 2;
    const int first_loop = (2 + 5 + 3 + 2 + 2 + 2) * 16 + 15 * 3 + 2;
    const int second_loop = (4 + 5 + 2 + 2) * 16 + 15 * 3 + 2;
    const int total_cycles = pre_loop + first_loop + second_loop + 7;

    EXPECT_EQ(total_cycles, run_until_brk());
    EXPECT_EQ(expected, registers);

    // Written in first loop
    for (uint8_t i = 0; i < 0x10; ++i) {
        EXPECT_EQ(i, mmu.read_byte(0x0200 + i));
    }
    // Written in second loop
    for (uint8_t i = 0; i < 0x10; ++i) {
        EXPECT_EQ(0x0F - i, mmu.read_byte(0x0210 + i));
    }
    // Verify stack content
    // BRK pushes three bytes (PC + 2 and P) and overwrites what the first loop
    // wrote.
    EXPECT_EQ(0x06, mmu.read_byte(kStackOffset + 0xFF));
    EXPECT_EQ(0x18 + 2, mmu.read_byte(kStackOffset + 0xFE));
    EXPECT_EQ(static_cast<uint8_t>(C_FLAG | Z_FLAG) | B_FLAG,
            mmu.read_byte(kStackOffset + 0xFD));

    for (uint8_t i = 3; i < 0x10; ++i) {
        EXPECT_EQ(i, mmu.read_byte(kStackOffset + 0xFF - i));
    }
}

TEST_F(CpuIntegrationTest, nmi) {
    // Address  Hexdump   Dissassembly
    // -------------------------------
    // $0600    a2 00     LDX #$00
    // $0602    a0 00     LDY #$00
    // $0604    e8        INX
    // $0605    4c 04 06  JMP $0604
    // nmi:
    // $1000    c8        INY
    // $1001    40        RTI
    load_hex_dump(0x0600,
            {0xa2, 0x00, 0xa0, 0x00, 0xe8, 0x4c, 0x04, 0x06, 0x00, 0x00});
    load_hex_dump(0x1000, {0xc8, 0x40, 0x00, 0x00});

    set_reset_address(0x0600);
    set_break_address(0xDEAD);
    set_nmi_address(0x1000);

    expected.x = 0x05;
    expected.y = 0x00;
    expected.pc = 0x0604;

    cpu->reset();
    // Run the loop 5 times to increase x
    step_execution(2 + 2 + (2 + 3) * 5); // ldx + ldy + (inx + jmp) * 5
    EXPECT_EQ(expected, registers);

    // Before jmp is finised, trigger an nmi
    step_execution(2 + 3 - 1); // inx + jmp - 1
    cpu->set_nmi(true);

    // jmp should finish before jumping to nmi
    expected.x = 0x06;
    step_execution(1);
    EXPECT_EQ(expected, registers);

    // Run rti subroutine and jump back
    expected.y = 0x01;
    expected.pc = 0x0604;
    expected.p |= FLAG_5;
    step_execution(7 + 2 + 6); // nmi + iny + rti
    EXPECT_EQ(expected, registers);
}

} // namespace
