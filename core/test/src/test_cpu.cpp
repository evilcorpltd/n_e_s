#include "cast_helpers.h"
#include "cpu_test.h"
#include "icpu_helpers.h"
#include "opcode.h"

using namespace n_e_s::core;
using namespace n_e_s::core::test;

using testing::Return;

namespace {

const uint16_t kStackOffset = 0x0100;
const uint16_t kResetAddress = 0xFFFC;
const uint16_t kBrkAddress = 0xFFFE;
const uint16_t kNmiAddress = 0xFFFA;

TEST_F(CpuTest, reset) {
    expected.pc = 0xDEAD;
    EXPECT_CALL(mmu, read_byte(kResetAddress)).WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kResetAddress + 1u)).WillOnce(Return(0xDE));

    cpu->reset();

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, reset_clears_pipeline) {
    stage_instruction(SEC);
    EXPECT_CALL(mmu, read_byte(kResetAddress)).WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kResetAddress + 1u)).WillOnce(Return(0xDE));
    EXPECT_CALL(mmu, read_byte(0xDEAD)).WillOnce(Return(0x00));
    expected.pc = 0xDEAD + 1;

    cpu->execute(); // Stage things for execution.
    cpu->reset();
    cpu->execute(); // Should read an opcode from 0xDEAD and not execute what's
                    // been staged.

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, state_is_initialized) {
    const auto state = cpu->state();

    EXPECT_FALSE(state.current_opcode.has_value());
    EXPECT_EQ(0u, state.start_pc);
    EXPECT_EQ(0u, state.start_cycle);
    EXPECT_EQ(0u, state.cycle);
}

TEST_F(CpuTest, state_opcode_returns_instruction) {
    registers.pc = 0x1234;
    stage_instruction(PHP);
    step_execution(1);

    const CpuState state = cpu->state();
    const auto opcode = state.current_opcode;

    EXPECT_EQ(0u, state.start_cycle);
    EXPECT_EQ(0x1234, state.start_pc);
    EXPECT_EQ(1u, state.cycle);

    EXPECT_TRUE(opcode.has_value());
    EXPECT_EQ(Instruction::PhpImplied, opcode->instruction);
    EXPECT_EQ(Family::PHP, opcode->family);
    EXPECT_EQ(AddressMode::Implied, opcode->address_mode);

    // Only cycle should change
    step_execution(1);
    const CpuState next_state = cpu->state();
    EXPECT_EQ(0u, next_state.start_cycle);
    EXPECT_EQ(0x1234, next_state.start_pc);
    EXPECT_EQ(2u, next_state.cycle);
}

TEST_F(CpuTest, unsupported_instruction) {
    stage_instruction(0xFF);

    EXPECT_THROW(step_execution(1), std::logic_error);
}

TEST_F(CpuTest, nmi) {
    registers.pc = 0x1234;
    cpu->set_nmi(true);

    expected.sp -= 2 + 1; // 1 word and 1 byte

    // Dummy reads
    EXPECT_CALL(mmu, read_byte(0x1234)).Times(2);

    expected.pc = 0x5678; // nmi vector

    // First the return address is pushed and then the registers.
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, 0x12));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 1, 0x34));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 2, registers.p));

    // Read nmi vector 0x5678
    EXPECT_CALL(mmu, read_byte(kNmiAddress)).WillOnce(Return(0x78));
    EXPECT_CALL(mmu, read_byte(kNmiAddress + 1)).WillOnce(Return(0x56));

    step_execution(7);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, brk) {
    registers.pc = 0x1234;
    stage_instruction(BRK);
    expected.pc = 0xDEAD;

    expected.sp -= 2 + 1; // 1 word and 1 byte

    // Dummy read
    EXPECT_CALL(mmu, read_byte(0x1235));

    // First the return address is pushed and then the registers.
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, 0x12));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 1, 0x36));
    EXPECT_CALL(mmu,
            write_byte(kStackOffset + registers.sp - 2, registers.p | B_FLAG));

    EXPECT_CALL(mmu, read_byte(kBrkAddress)).WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kBrkAddress + 1)).WillOnce(Return(0xDE));

    step_execution(7);

    EXPECT_EQ(expected, registers);
}

// ASL
TEST_F(CpuTest, asl_acc_ignores_carry) {
    stage_instruction(ASL_ACC);
    registers.p = C_FLAG;
    registers.a = 0b00110011;
    expected.a = 0b01100110;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, asl_acc_set_c_and_z_flags_clears_n) {
    stage_instruction(ASL_ACC);
    registers.a = 0b10000000;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, php_sets_b_flag) {
    stage_instruction(PHP);
    registers.sp = 0x0A;
    registers.p = N_FLAG | Z_FLAG;

    expected.sp = 0x09;
    expected.p = registers.p;

    EXPECT_CALL(
            mmu, write_byte(kStackOffset + registers.sp, registers.p | B_FLAG));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bpl_branch_not_taken) {
    stage_instruction(BPL);
    expected.p = registers.p = N_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, plp_clears_b_and_sets_bit_5) {
    stage_instruction(PLP);
    registers.sp = 0x0A;
    registers.p = 0xBB;

    expected.sp = registers.sp + static_cast<uint8_t>(1u);
    expected.p = static_cast<uint8_t>(FLAG_5 | C_FLAG) | N_FLAG;

    EXPECT_CALL(mmu, read_byte(kStackOffset + expected.sp))
            .WillOnce(Return(static_cast<uint8_t>(B_FLAG | C_FLAG) | N_FLAG));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, clc) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLC);
    expected.p &= static_cast<uint8_t>(~C_FLAG);

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jsr) {
    registers.pc = 0x1234;
    stage_instruction(JSR);
    expected.pc = 0xDEAD;
    expected.sp -= 2; // pc (2 bytes)

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, 0x12));
    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp - 1u, 0x36));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2u)).WillOnce(Return(0xDE));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bmi_branch_not_taken) {
    stage_instruction(BMI);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sec) {
    stage_instruction(SEC);
    expected.p |= C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_shifts) {
    stage_instruction(LSR_ACC);
    registers.a = 0b01001000;
    expected.a = 0b00100100;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_z_flag) {
    stage_instruction(LSR_ACC);
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_flag) {
    stage_instruction(LSR_ACC);
    registers.a = 0b00000011;
    expected.a = 0b00000001;
    expected.p = C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_sets_c_and_z_flags) {
    stage_instruction(LSR_ACC);
    registers.a = 0b00000001;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, lsr_a_clears_c_z_n_flags) {
    stage_instruction(LSR_ACC);
    registers.a = 0b00000010;
    registers.p = static_cast<uint8_t>(Z_FLAG | C_FLAG) | N_FLAG;
    expected.a = 0b00000001;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pha) {
    stage_instruction(PHA);
    registers.sp = 0x05;
    registers.a = 0x84;

    EXPECT_CALL(mmu, write_byte(kStackOffset + registers.sp, registers.a));

    expected.sp = 0x04;
    expected.a = registers.a;

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jmp) {
    stage_instruction(JMP);
    expected.pc = 0x1234;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2u)).WillOnce(Return(0x12));

    step_execution(3);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jmp_indirect) {
    registers.pc = 0x1000;
    stage_instruction(JMP_IND);
    expected.pc = 0x6789;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2u)).WillOnce(Return(0x12));
    EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(0x89));
    EXPECT_CALL(mmu, read_byte(0x1235)).WillOnce(Return(0x67));

    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, jmp_indirect_fetches_address_from_same_page) {
    registers.pc = 0x1000;
    stage_instruction(JMP_IND);
    expected.pc = 0x6789;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0xFF));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2u)).WillOnce(Return(0x12));
    // Address is fetched from 0x12FF and 0x1200 instead of 0x12FF and 0x1300
    EXPECT_CALL(mmu, read_byte(0x12FF)).WillOnce(Return(0x89));
    EXPECT_CALL(mmu, read_byte(0x1200)).WillOnce(Return(0x67));

    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bvc_branch_not_taken) {
    stage_instruction(BVC);
    expected.p = registers.p = V_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pla_sets_z_clears_n) {
    stage_instruction(PLA);
    registers.sp = 0x0A;
    registers.a = 0xBB;
    registers.p = N_FLAG;

    expected.sp = registers.sp + static_cast<uint8_t>(1u);
    expected.a = 0x00;
    expected.p = Z_FLAG;

    EXPECT_CALL(mmu, read_byte(kStackOffset + expected.sp))
            .WillOnce(Return(0x00));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, pla_sets_n_clears_z) {
    stage_instruction(PLA);
    registers.sp = 0x0A;
    registers.a = 0xBB;
    registers.p = Z_FLAG;

    expected.sp = registers.sp + static_cast<uint8_t>(1u);
    expected.a = 0x92;
    expected.p = N_FLAG;

    EXPECT_CALL(mmu, read_byte(kStackOffset + expected.sp))
            .WillOnce(Return(0x92));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cli) {
    stage_instruction(CLI);
    registers.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, rts) {
    registers.pc = 0x1234;
    stage_instruction(RTS);
    registers.sp = 0x0A;
    expected.sp = 0x0C;
    expected.pc = 0xDEAD + 1;

    // Dummy read
    EXPECT_CALL(mmu, read_byte(0x1235));

    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 1))
            .WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 2))
            .WillOnce(Return(0xDE));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, rti) {
    registers.pc = 0x1234;
    stage_instruction(RTI);
    registers.sp = 0x0A;
    expected.p = static_cast<uint8_t>(Z_FLAG | C_FLAG) | FLAG_5;
    expected.sp = registers.sp + static_cast<uint8_t>(3);
    expected.pc = 0xDEAD;

    // Dummy read
    EXPECT_CALL(mmu, read_byte(0x1235));

    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 1))
            .WillOnce(Return(static_cast<uint8_t>(Z_FLAG | C_FLAG) | B_FLAG));
    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 2))
            .WillOnce(Return(0xAD));
    EXPECT_CALL(mmu, read_byte(kStackOffset + registers.sp + 3))
            .WillOnce(Return(0xDE));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bvs_branch_not_taken) {
    stage_instruction(BVS);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sei) {
    stage_instruction(SEI);
    expected.p |= I_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bcc_branch_not_taken) {
    stage_instruction(BCC);
    expected.p = registers.p = C_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// LDA indirect indexed
TEST_F(CpuTest, lda_indirect_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x0D;
    registers.p = Z_FLAG;
    expected.a = 0xE0;
    expected.p = N_FLAG;

    stage_instruction(LDA_INDINX);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x0D)).WillOnce(Return(0xE0));
    step_execution(5);

    EXPECT_EQ(expected, registers);
}

// LAX
TEST_F(CpuTest, lax_indexed_indirect) {
    registers.pc = expected.pc = 0x4322;
    registers.x = 0xED;
    expected.x = 0x42; // X is overwritten, so we can not use the test fixture
    expected.a = 0x42;
    stage_instruction(LAX_INXIND);
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_byte(0xAB)).WillOnce(Return(0x68)); // Dummy read
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED))).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED + 1u)))
            .WillOnce(Return(0x12));
    EXPECT_CALL(mmu, read_byte(0x1234)).WillOnce(Return(0x42));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, lax_indirect_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0x0D;
    expected.a = 0x42;
    expected.x = 0x42;

    stage_instruction(LAX_INDINX);
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x42));
    EXPECT_CALL(mmu, read_byte(0x42)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(0x43)).WillOnce(Return(0x12));

    EXPECT_CALL(mmu, read_byte(0x1234 + 0x0D)).WillOnce(Return(0x42));
    step_execution(5);

    EXPECT_EQ(expected, registers);
}

// BCS
TEST_F(CpuTest, bcs_branch_not_taken) {
    stage_instruction(BCS);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, clv) {
    stage_instruction(CLV);
    registers.p |= V_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, bne_branch_not_taken) {
    stage_instruction(BNE);
    expected.p = registers.p = Z_FLAG;
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, cld) {
    expected.p = registers.p = 0xFF;

    stage_instruction(CLD);
    expected.p &= static_cast<uint8_t>(~D_FLAG);

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// NOP
TEST_F(CpuTest, nop80_immediated) {
    stage_instruction(NOP_IMM80);
    expected.pc += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, nop04_zero) {
    stage_instruction(NOP_ZERO04);
    expected.pc += 1;
    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0xCD));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, nop0c_abs) {
    stage_instruction(NOP_ABS0C);
    expected.pc += 2;
    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0xCD));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2)).WillOnce(Return(0xEF));

    step_execution(4);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, nop44_zero) {
    stage_instruction(NOP_ZERO44);
    expected.pc += 1;
    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0xCD));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}
// TODO(robinlinden): This test doesnt test what it says it tests.
TEST_F(CpuTest, nop64_zero) {
    stage_instruction(NOP_ZERO44);
    expected.pc += 1;
    EXPECT_CALL(mmu, read_byte(registers.pc + 1)).WillOnce(Return(0xCD));

    step_execution(3);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, beq_branch_not_taken) {
    stage_instruction(BEQ);
    expected.pc += 1;
    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, inc_zerox_increments) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;

    stage_instruction(INC_ZEROX);
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(0x44)).WillOnce(Return(0xCD)); // Dummy read
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x05));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x05)); // Dummy write
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x06));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

// DEC
TEST_F(CpuTest, dec_zerox_decrements) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;

    stage_instruction(DEC_ZEROX);
    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0x44));
    EXPECT_CALL(mmu, read_byte(0x44)).WillOnce(Return(0xCD)); // Dummy read
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0x44 + 0xED))).WillOnce(Return(0x05));
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x05)); // Dummy write
    EXPECT_CALL(mmu, write_byte(u16_to_u8(0x44 + 0xED), 0x04));

    step_execution(6);
    EXPECT_EQ(expected, registers);
}

// INX
TEST_F(CpuTest, inx_increments) {
    stage_instruction(INX);
    expected.x += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, inx_sets_n_flag) {
    stage_instruction(INX);
    registers.x = 127;
    expected.x = 128;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, inx_clears_n_flag) {
    stage_instruction(INX);
    registers.x = 255;
    registers.p |= N_FLAG;
    expected.x = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, inx_sets_z_flag) {
    stage_instruction(INX);
    registers.x = 255;
    expected.x = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, inx_clears_z_flag) {
    stage_instruction(INX);
    registers.x = 0;
    registers.p |= Z_FLAG;
    expected.x = 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// INY
TEST_F(CpuTest, iny_increments) {
    stage_instruction(INY);
    expected.y += 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_sets_n_flag) {
    stage_instruction(INY);
    registers.y = 127;
    expected.y = 128;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_clears_n_flag) {
    stage_instruction(INY);
    registers.y = 255;
    registers.p |= N_FLAG;
    expected.y = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_sets_z_flag) {
    stage_instruction(INY);
    registers.y = 255;
    expected.y = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, iny_clears_z_flag) {
    stage_instruction(INY);
    registers.y = 0;
    registers.p |= Z_FLAG;
    expected.y = 1;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// SED
TEST_F(CpuTest, sed) {
    stage_instruction(SED);
    expected.p |= D_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_abs_x_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_ABSX);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2u)).WillOnce(Return(0x12));
    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0xED, 0x07));

    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_abs_y_indexed) {
    registers.pc = expected.pc = 0x4321;
    registers.y = expected.y = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_ABSY);

    expected.pc += 2;

    EXPECT_CALL(mmu, read_byte(registers.pc + 1u)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(registers.pc + 2u)).WillOnce(Return(0x12));
    EXPECT_CALL(mmu, read_byte(0x1234 + 0xED - 0x0100))
            .WillOnce(Return(0xDEAD));
    EXPECT_CALL(mmu, write_byte(0x1234 + 0xED, 0x07));

    step_execution(5);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indexed_indirect) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0xED;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_byte(0xAB)).WillOnce(Return(0x00)); // Dummy read
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED))).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + 0xED + 1u)))
            .WillOnce(Return(0x12));
    EXPECT_CALL(mmu, write_byte(0x1234, 0x07));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, sta_indexed_indirect_handles_wraparound) {
    registers.pc = expected.pc = 0x4321;
    registers.x = expected.x = 0x00;
    registers.a = expected.a = 0x07;

    stage_instruction(STA_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xFF));
    EXPECT_CALL(mmu, read_byte(0xFF)).WillOnce(Return(0x00)); // Dummy read
    EXPECT_CALL(mmu, read_byte(0xFF)).WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(0x00)).WillOnce(Return(0x12));
    EXPECT_CALL(mmu, write_byte(0x1234, 0x07));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, txs) {
    registers.x = expected.x = 0xAA;

    stage_instruction(TXS);
    expected.sp = 0xAA;

    step_execution(2);

    EXPECT_EQ(expected, registers);
}

// SAX
TEST_F(CpuTest, sax_indexed_indirect) {
    registers.pc = expected.pc = 0x4321;
    registers.a = expected.a = 0b10101010;
    registers.x = expected.x = 0b00001111;

    stage_instruction(SAX_INXIND);

    expected.pc += 1;

    EXPECT_CALL(mmu, read_byte(0x4322)).WillOnce(Return(0xAB));
    EXPECT_CALL(mmu, read_byte(0xAB)).WillOnce(Return(0x00)); // Dummy read
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + registers.x)))
            .WillOnce(Return(0x34));
    EXPECT_CALL(mmu, read_byte(u16_to_u8(0xAB + registers.x + 1u)))
            .WillOnce(Return(0x12));
    EXPECT_CALL(mmu, write_byte(0x1234, 0b00001010));

    step_execution(6);

    EXPECT_EQ(expected, registers);
}

// DEY
TEST_F(CpuTest, dey_decrements) {
    stage_instruction(DEY);
    registers.y = 3;
    expected.y = 2;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_sets_n_flag) {
    stage_instruction(DEY);
    registers.y = 0;
    expected.y = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_clears_n_flag) {
    stage_instruction(DEY);
    registers.y = 128;
    registers.p |= N_FLAG;
    expected.y = 127;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_sets_z_flag) {
    stage_instruction(DEY);
    registers.y = 1;
    expected.y = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dey_clears_z_flag) {
    stage_instruction(DEY);
    registers.y = 0;
    registers.p |= Z_FLAG;
    expected.y = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// DEX
TEST_F(CpuTest, dex_decrements) {
    stage_instruction(DEX);
    registers.x = 3;
    expected.x = 2;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_sets_n_flag) {
    stage_instruction(DEX);
    registers.x = 0;
    expected.x = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_clears_n_flag) {
    stage_instruction(DEX);
    registers.x = 128;
    registers.p |= N_FLAG;
    expected.x = 127;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_sets_z_flag) {
    stage_instruction(DEX);
    registers.x = 1;
    expected.x = 0;
    expected.p |= Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, dex_clears_z_flag) {
    stage_instruction(DEX);
    registers.x = 0;
    registers.p |= Z_FLAG;
    expected.x = 255;
    expected.p |= N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// ROL, ACC
TEST_F(CpuTest, rol_a_rotates) {
    stage_instruction(ROL_ACC);
    registers.a = 0b00110011;
    expected.a = 0b01100110;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_all_zero_no_change) {
    stage_instruction(ROL_ACC);
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_c_set_clears_c_and_z_flags) {
    stage_instruction(ROL_ACC);
    registers.a = 0b00000000;
    registers.p = C_FLAG;
    expected.a = 0b00000001;
    expected.p = 0;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_set_c_and_z_flags_clears_n) {
    stage_instruction(ROL_ACC);
    registers.a = 0b10000000;
    registers.p = N_FLAG;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, rol_a_clear_neg_retain_c) {
    stage_instruction(ROL_ACC);
    registers.a = 0b10000000;
    registers.p = N_FLAG | C_FLAG;
    expected.a = 0b00000001;
    expected.p = C_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

// ROR, ACC
TEST_F(CpuTest, ror_a_rotates) {
    stage_instruction(ROR_ACC);
    registers.a = 0b11001100;
    expected.a = 0b01100110;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ror_a_all_zero_no_change) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000000;
    registers.p = Z_FLAG;
    expected.a = 0b00000000;
    expected.p = Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

TEST_F(CpuTest, ror_a_c_set_clears_c_and_sets_z_flags) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000000;
    registers.p = C_FLAG;
    expected.a = 0b10000000;
    expected.p = N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, ror_a_set_c_and_z_flags) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000001;
    registers.p = 0;
    expected.a = 0b00000000;
    expected.p = C_FLAG | Z_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}
TEST_F(CpuTest, ror_a_set_neg_retain_c) {
    stage_instruction(ROR_ACC);
    registers.a = 0b00000001;
    registers.p = C_FLAG;
    expected.a = 0b10000000;
    expected.p = C_FLAG | N_FLAG;

    step_execution(2);
    EXPECT_EQ(expected, registers);
}

} // namespace
