#include "core/opcode.h"

#include <gtest/gtest.h>
#include <string>

using namespace n_e_s::core;

namespace {

TEST(Opcode, Family) {
    EXPECT_EQ("Invalid", to_string(Family::Invalid));
    EXPECT_EQ("BRK", to_string(Family::BRK));
    EXPECT_EQ("ASL", to_string(Family::ASL));
    EXPECT_EQ("INC", to_string(Family::INC));
    EXPECT_EQ("PHP", to_string(Family::PHP));
    EXPECT_EQ("BPL", to_string(Family::BPL));
    EXPECT_EQ("CLC", to_string(Family::CLC));
    EXPECT_EQ("BIT", to_string(Family::BIT));
    EXPECT_EQ("PLP", to_string(Family::PLP));
    EXPECT_EQ("AND", to_string(Family::AND));
    EXPECT_EQ("RTI", to_string(Family::RTI));
    EXPECT_EQ("JSR", to_string(Family::JSR));
    EXPECT_EQ("BMI", to_string(Family::BMI));
    EXPECT_EQ("SEC", to_string(Family::SEC));
    EXPECT_EQ("LSR", to_string(Family::LSR));
    EXPECT_EQ("PHA", to_string(Family::PHA));
    EXPECT_EQ("JMP", to_string(Family::JMP));
    EXPECT_EQ("BVC", to_string(Family::BVC));
    EXPECT_EQ("CLI", to_string(Family::CLI));
    EXPECT_EQ("ADC", to_string(Family::ADC));
    EXPECT_EQ("PLA", to_string(Family::PLA));
    EXPECT_EQ("RTS", to_string(Family::RTS));
    EXPECT_EQ("BVS", to_string(Family::BVS));
    EXPECT_EQ("SEI", to_string(Family::SEI));
    EXPECT_EQ("STY", to_string(Family::STY));
    EXPECT_EQ("STA", to_string(Family::STA));
    EXPECT_EQ("STX", to_string(Family::STX));
    EXPECT_EQ("TXS", to_string(Family::TXS));
    EXPECT_EQ("BCC", to_string(Family::BCC));
    EXPECT_EQ("LDX", to_string(Family::LDX));
    EXPECT_EQ("LDY", to_string(Family::LDY));
    EXPECT_EQ("LDA", to_string(Family::LDA));
    EXPECT_EQ("BCS", to_string(Family::BCS));
    EXPECT_EQ("CLV", to_string(Family::CLV));
    EXPECT_EQ("BNE", to_string(Family::BNE));
    EXPECT_EQ("CLD", to_string(Family::CLD));
    EXPECT_EQ("CPX", to_string(Family::CPX));
    EXPECT_EQ("NOP", to_string(Family::NOP));
    EXPECT_EQ("INX", to_string(Family::INX));
    EXPECT_EQ("INY", to_string(Family::INY));
    EXPECT_EQ("CPY", to_string(Family::CPY));
    EXPECT_EQ("CMP", to_string(Family::CMP));
    EXPECT_EQ("DEC", to_string(Family::DEC));
    EXPECT_EQ("BEQ", to_string(Family::BEQ));
    EXPECT_EQ("SED", to_string(Family::SED));
    EXPECT_EQ("TYA", to_string(Family::TYA));
    EXPECT_EQ("TAY", to_string(Family::TAY));
    EXPECT_EQ("TAX", to_string(Family::TAX));
    EXPECT_EQ("TSX", to_string(Family::TSX));
    EXPECT_EQ("TXA", to_string(Family::TXA));
    EXPECT_EQ("DEY", to_string(Family::DEY));
    EXPECT_EQ("DEX", to_string(Family::DEX));
    EXPECT_EQ("EOR", to_string(Family::EOR));
    EXPECT_EQ("ROL", to_string(Family::ROL));
    EXPECT_EQ("ROR", to_string(Family::ROR));
    EXPECT_EQ("ORA", to_string(Family::ORA));
    EXPECT_EQ("SBC", to_string(Family::SBC));
}

} // namespace
