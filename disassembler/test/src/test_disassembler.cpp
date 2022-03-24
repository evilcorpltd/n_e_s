#include "nes/core/imos6502.h"
#include "nes/core/opcode.h"
#include "nes/disassembler.h"

#include "nes/core/test/fake_mmu.h"

#include <gtest/gtest.h>

using namespace n_e_s::dis;

namespace {

class DisassemblerFixture : public testing::Test {
public:
    n_e_s::core::test::FakeMmu mmu;
    n_e_s::core::CpuRegisters reg;

    void load_hex_dump(uint16_t address, const std::vector<uint8_t> &data) {
        for (auto d : data) {
            mmu.write_byte(address++, d);
        }
    }
};

TEST(get_arg_count, returns_correct_count) {
    EXPECT_EQ(0, get_arg_count(n_e_s::core::AddressMode::Implied));
    EXPECT_EQ(0, get_arg_count(n_e_s::core::AddressMode::Accumulator));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::Immediate));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::Zeropage));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::ZeropageX));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::ZeropageY));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::Relative));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::IndexedIndirect));
    EXPECT_EQ(1, get_arg_count(n_e_s::core::AddressMode::IndirectIndexed));
    EXPECT_EQ(2, get_arg_count(n_e_s::core::AddressMode::Absolute));
    EXPECT_EQ(2, get_arg_count(n_e_s::core::AddressMode::AbsoluteX));
    EXPECT_EQ(2, get_arg_count(n_e_s::core::AddressMode::AbsoluteY));
    EXPECT_EQ(2, get_arg_count(n_e_s::core::AddressMode::Indirect));
}

// No args
TEST_F(DisassemblerFixture, disassemble_implied) {
    load_hex_dump(0x1234, {n_e_s::core::PhpImplied, 0x00, 0x01});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("PHP", res);
}

TEST_F(DisassemblerFixture, disassemble_accumulator) {
    load_hex_dump(0x1234, {n_e_s::core::AslAccumulator, 0x00, 0x01});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("ASL A", res);
}

// 1 arg
TEST_F(DisassemblerFixture, disassemble_immediate) {
    load_hex_dump(0x1234, {n_e_s::core::AndImmediate, 0xAB, 0x01});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("AND #$AB", res);
}

TEST_F(DisassemblerFixture, disassemble_zeropage) {
    load_hex_dump(0x1234, {n_e_s::core::BitZeropage, 0xAB, 0x01});
    load_hex_dump(0xAB, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("BIT $AB = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_zeropagex) {
    reg.x = 0x02;
    load_hex_dump(0x1234, {n_e_s::core::SbcZeropageX, 0xAB, 0x01});
    load_hex_dump(0xAD, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("SBC $AB,X @ AD = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_zeropagey) {
    reg.y = 0x02;
    load_hex_dump(0x1234, {n_e_s::core::LaxZeropageY, 0xAB, 0x01});
    load_hex_dump(0xAD, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("LAX $AB,Y @ AD = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_relative) {
    load_hex_dump(0x1234, {n_e_s::core::BccRelative, 0x12, 0x01});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("BCC $1248", res);
}

TEST_F(DisassemblerFixture, disassemble_indexed_indirect) {
    reg.x = 0x02;
    load_hex_dump(0x1234, {n_e_s::core::AdcIndirectX, 0x12, 0x01});
    load_hex_dump(0x12 + 0x02, {0x60});
    load_hex_dump(0x12 + 0x02 + 0x01, {0xCD});
    load_hex_dump(0xCD60, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("ADC ($12,X) @ 14 = CD60 = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_indirect_indexed) {
    reg.y = 0x02;
    load_hex_dump(0x1234, {n_e_s::core::CmpIndirectY, 0x12, 0x01});
    load_hex_dump(0x12 + 0x01, {0x60});
    load_hex_dump(0x12, {0xCD});
    load_hex_dump(0x60CD, {0xAB});
    load_hex_dump(0x60CD + 0x02, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("CMP ($12),Y = 60CD @ 60CF = 42", res);
}

// 2 args
TEST_F(DisassemblerFixture, disassemble_absolute) {
    load_hex_dump(0x1234, {n_e_s::core::AslAbsolute, 0x78, 0x56});
    load_hex_dump(0x5678, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("ASL $5678 = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_absolute_jmp) {
    load_hex_dump(0x1234, {n_e_s::core::JmpAbsolute, 0x78, 0x56});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("JMP $5678", res);
}

TEST_F(DisassemblerFixture, disassemble_absolutex) {
    reg.x = 0x03;
    load_hex_dump(0x1234, {n_e_s::core::LdaAbsoluteX, 0x78, 0x56});
    load_hex_dump(0x5678 + 0x03, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("LDA $5678,X @ 567B = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_absolutey) {
    reg.y = 0x03;
    load_hex_dump(0x1234, {n_e_s::core::LdxAbsoluteY, 0x78, 0x56});
    load_hex_dump(0x5678 + 0x03, {0x42});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("LDX $5678,Y @ 567B = 42", res);
}

TEST_F(DisassemblerFixture, disassemble_indirect) {
    load_hex_dump(0x1234, {n_e_s::core::JmpIndirect, 0x78, 0x56});
    load_hex_dump(0x5678, {0x42});
    load_hex_dump(0x5679, {0x43});
    const std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("JMP ($5678) = 4342", res);
}

TEST_F(DisassemblerFixture, does_not_read_ppu_memory) {
    // 0x2007 is mapped to PpuData.
    load_hex_dump(0x1234, {n_e_s::core::StaAbsolute, 0x07, 0x20});
    // 0x2002 is mapped to PpuStatus.
    load_hex_dump(0xABCD, {n_e_s::core::StaAbsolute, 0x02, 0x20});

    std::string res = disassemble(0x1234, mmu, reg);
    EXPECT_EQ("STA $2007 = 00", res);

    res = disassemble(0xABCD, mmu, reg);
    EXPECT_EQ("STA $2002 = 00", res);
}

} // namespace
